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

"""Posix client"""
from abc import ABC

from fnruntime.rpc import sdk, rpc_server
from yr.invoke_client.invoke_client import InvokeClient
from yr.rpc import core_service_pb2
from yr.runtime.task_spec import TaskSpec, InvokeOptions


class PosixClient(InvokeClient, ABC):
    """Posix invoke client"""

    def create(self, task: TaskSpec):
        """create instance"""
        if task.invoke_options is not None:
            scheduling_ops = task.invoke_options.to_pb()
            create_ops = {'Concurrency': f"{task.invoke_options.concurrency}"}
            labels = task.invoke_options.labels
        else:
            scheduling_ops = core_service_pb2.SchedulingOptions()
            create_ops = {'Concurrency': f"{InvokeOptions().concurrency}"}
            labels = []
        instance_id = task.instance_id.id if task.instance_id else None

        request = core_service_pb2.CreateRequest(function=task.function_info.function_name,
                                                 schedulingOps=scheduling_ops,
                                                 createOptions=create_ops,
                                                 requestID=task.task_id,
                                                 traceID=task.trace_id,
                                                 labels=labels,
                                                 args=task.args,
                                                 designatedInstanceID=instance_id)
        sdk.create(request, task.callback)

    def invoke(self, task: TaskSpec):
        """invoke"""
        request = core_service_pb2.InvokeRequest(function=task.function_info.function_name,
                                                 requestID=task.task_id,
                                                 instanceID=task.instance_id.id,
                                                 traceID=task.trace_id,
                                                 args=task.args,
                                                 returnObjectIDs=task.object_ids)
        sdk.invoke(request, task.callback)

    def kill(self, task: TaskSpec):
        """kill"""
        sdk.kill(task.instance_id.id, task.signal, task.request_id, task.callback, task.invoke_timeout)

    def exit(self):
        """exit"""
        sdk.exit()

    def save_state(self, state) -> str:
        """save state"""

        return sdk.save_state(state)

    def load_state(self, checkpoint_id) -> bytes:
        """load state"""

        return sdk.load_state(checkpoint_id)

    def clear(self):
        """clear"""


def start_server(address, logger) -> int:
    """
    start server for communicating with the kernel

    Args:
        address: bind address
        logger: logger that has been initialized
    """
    from fnruntime import log
    log.init_log("", logger)
    return rpc_server.driver_serve(address)
