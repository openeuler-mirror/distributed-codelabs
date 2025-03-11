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
http client
This is a client of yr cluster in outCluster
"""
import logging
from abc import ABC

from yr.invoke_client.async_http_client import AsyncHttpClient, HttpTask
from yr.invoke_client.invoke_client import InvokeClient, create_headers
from yr.rpc import core_service_pb2
from yr.runtime.task_spec import TaskSpec, InvokeOptions

_CLUSTER_PORT = 31220
_MAX_HTTP = 1024
_CONNECT_TIMEOUT = 5
_READ_TIMEOUT = 900
_GETCON_TIMEOUT = 295

_INSTANCE_INVOKE_TYPE = "invoke"
_INSTANCE_TERMINATE_TYPE = "terminate"
_INSTANCE_INVOKE_SOURCE = "yrapi"

_INVOKE_PATH = "/serverless/v1/functions/"
_OUTCLUSTER_CREATE_PATH = "/serverless/v1/posix/instance/create"
_OUTCLUSTER_INVOKE_PATH = "/serverless/v1/posix/instance/invoke"
_OUTCLUSTER_KILL_PATH = "/serverless/v1/posix/instance/kill"

logger = logging.getLogger(__name__)


class HttpInvokeClient(InvokeClient, ABC):
    """
    http invoke client for api invoke
    """

    def __init__(self):
        self.http_client = None

    def init(self, server_address, timeout):
        """
        init client
        """
        self.http_client = AsyncHttpClient()
        self.http_client.init(address=server_address, timeout=timeout)

    def create(self, task: TaskSpec):
        """create instance"""
        logger.debug(f"create task:{task.task_id} ,trace_id:{task.trace_id}")
        if task.invoke_options is not None:
            scheduling_ops = task.invoke_options.to_pb()
            create_ops = {'Concurrency': f"{task.invoke_options.concurrency}"}
            labels = task.invoke_options.labels
        else:
            scheduling_ops = core_service_pb2.SchedulingOptions()
            create_ops = {'Concurrency': f"{InvokeOptions().concurrency}"}
            labels = []
        instance_id = task.instance_id.id if task.instance_id else None
        payload = core_service_pb2.CreateRequest(function=task.function_info.function_name,
                                                 schedulingOps=scheduling_ops,
                                                 createOptions=create_ops,
                                                 requestID=task.task_id,
                                                 traceID=task.trace_id,
                                                 labels=labels,
                                                 args=task.args,
                                                 designatedInstanceID=instance_id).SerializeToString()

        url = _OUTCLUSTER_CREATE_PATH
        headers = create_headers(url, task.function_info, payload, task.task_id)
        self.http_client.submit_invoke_request(
            HttpTask(url=url, headers=headers, payload=payload, read_timeout=task.invoke_timeout, task_id=task.task_id),
            task.callback)

    def invoke(self, task: TaskSpec):
        """invoke"""
        logger.debug(f"invoke task:{task.task_id} ,trace_id:{task.trace_id}")
        req = core_service_pb2.InvokeRequest(function=task.function_info.function_name,
                                             requestID=task.task_id,
                                             instanceID=task.instance_id.id,
                                             traceID=task.trace_id,
                                             args=task.args,
                                             returnObjectIDs=task.object_ids)
        payload = req.SerializeToString()
        url = _OUTCLUSTER_INVOKE_PATH
        headers = create_headers(url, task.function_info, payload, task.task_id)
        self.http_client.submit_invoke_request(
            HttpTask(url=url, headers=headers, payload=payload, read_timeout=task.invoke_timeout, task_id=task.task_id),
            task.callback)

    def kill(self, task: TaskSpec):
        """kill"""
        # instance_id.id is a future object and will wait for instance creation to complete
        req = core_service_pb2.KillRequest(instanceID=task.instance_id.id,
                                           signal=task.signal, payload=task.request_id.encode(encoding='utf-8'))

        logger.debug(
            f"send kill, id: {task.instance_id}, signal: {task.signal}, task: {task.task_id}")
        url = _OUTCLUSTER_KILL_PATH
        payload = req.SerializeToString()
        headers = create_headers(url, task.function_info, payload, task.task_id)
        self.http_client.submit_invoke_request(
            HttpTask(url=url, headers=headers, payload=payload, read_timeout=task.invoke_timeout, task_id=task.task_id),
            task.callback)

    def exit(self):
        """exit"""
        raise RuntimeError("Not support exit out of cluster")

    def save_state(self, state):
        """save state"""
        raise RuntimeError("Not support save state out of cluster")

    def load_state(self, checkpoint_id) -> bytes:
        """load state"""
        raise RuntimeError("Not support load state out of cluster")

    def clear(self):
        """clear pool"""
        self.http_client.shutdown()
