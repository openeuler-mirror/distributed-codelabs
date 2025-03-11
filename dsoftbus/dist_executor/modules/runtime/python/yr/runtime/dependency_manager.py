#!/usr/bin/env python3
# coding=UTF-8
# Copyright (c) 2022 Huawei Technologies Co., Ltd
#
# This software is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#
# http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

"""
Dependency Manager
"""
import concurrent.futures
import functools
import logging
from dataclasses import dataclass
from typing import Union, List, Callable, Optional

from yr.object_ref import ObjectRef
from yr.instance_ref import InstanceRef
from yr.objref_counting_manager import ObjRefCountingManager
from yr.runtime.task_spec import TaskSpec
from yr.serialization import SerializeObject

_logger = logging.getLogger(__name__)


@dataclass
class TaskState:
    """TaskState"""
    task: TaskSpec
    object_dependencies: int
    instance_dependencies: int
    instance_error: Optional[BaseException]
    error: List[BaseException]


def resolve_dependency(task: TaskSpec, on_complete: Callable[[Optional[BaseException], List], None]):
    """resolve dependency"""
    object_dependencies = []
    instance_dependencies = []

    if task.instance_id is not None:
        instance_dependencies.append(task.instance_id)

    for arg in task.args:
        if not isinstance(arg, SerializeObject):
            continue
        for ref in arg.refs:
            if isinstance(ref, ObjectRef):
                object_dependencies.append(ref)

    ObjRefCountingManager().join_record(task.task_id, object_dependencies)

    if len(object_dependencies) == 0 and len(instance_dependencies) == 0:
        on_complete(None, [])
        return
    state = TaskState(task, len(object_dependencies), len(instance_dependencies), None, [])

    def callback(future: concurrent.futures.Future, dependency: Union[ObjectRef, InstanceRef]):
        err = future.exception() if future else None

        if isinstance(dependency, ObjectRef):
            state.object_dependencies -= 1
            if err:
                state.error.append(future.exception())
        else:
            state.instance_dependencies -= 1
            if not err:
                dependency.wait()
            else:
                state.instance_error = err

        if state.object_dependencies == 0 and state.instance_dependencies == 0:
            on_complete(state.instance_error, state.error)

    for ref in object_dependencies:
        ref.on_complete(functools.partial(callback, dependency=ref))
    for ref in instance_dependencies:
        ref.on_complete(functools.partial(callback, dependency=ref))
