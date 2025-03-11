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
"""Scheduler"""

from abc import abstractmethod, ABC
from typing import Optional, Iterable, List

from yr.runtime.instance import Instance
from yr.runtime.task_spec import TaskSpec


class Scorer(ABC):
    """abstract Scorer for scoring instances"""

    @staticmethod
    @abstractmethod
    def score(task: TaskSpec, instance: Instance) -> int:
        """scoring instance"""


class ConcurrencyScorer(Scorer):
    """Scorer for concurrency"""

    @staticmethod
    def score(task: TaskSpec, instance: Instance) -> int:
        """scoring instance"""
        if instance.task_count < instance.resource.concurrency:
            return 1
        return 0


class Scheduler(ABC):
    """abstract Scheduler"""

    @abstractmethod
    def schedule(self, task: TaskSpec, instances: Iterable[Instance]) -> Optional[Instance]:
        """select a instance"""


class NormalScheduler(Scheduler):
    """base scheduler implementation"""
    __slots__ = ["__scorers"]

    def __init__(self, scorers: List[Scorer] = None):
        if scorers is None:
            self.__scorers = []
        else:
            self.__scorers = scorers

    def schedule(self, task: TaskSpec, instances: Iterable[Instance]) -> Optional[Instance]:
        """select a instance"""
        max_score = 0
        max_instance = None
        for instance in instances:
            if not instance.is_activate:
                continue
            score = 0
            for scorer in self.__scorers:
                score += scorer.score(task, instance)
            if score > max_score:
                max_score = score
                max_instance = instance
        return max_instance
