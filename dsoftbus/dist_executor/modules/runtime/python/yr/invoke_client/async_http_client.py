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
async http client
"""
import atexit
import http
import threading
from concurrent.futures import ThreadPoolExecutor
from concurrent.futures import thread
import logging
from dataclasses import dataclass
import traceback

import urllib3
from urllib3 import HTTPConnectionPool, HTTPSConnectionPool

from yr.exception import YRInvokeError
from yr.config import ConfigManager

_CONNECT_TIMEOUT = 5
_READ_TIMEOUT = 900
_GETCON_TIMEOUT = 295

_logger = logging.getLogger(__name__)


@dataclass
class HttpTask:
    """http task"""
    task_id: str
    url: str
    headers: dict
    payload: str
    connect_timeout: int = _CONNECT_TIMEOUT
    read_timeout: int = _READ_TIMEOUT


class AsyncHttpClient:
    """
    async http client for frontend
    """

    def __init__(self):
        self.client = None
        self.pool = None
        self.timeout = None
        self._shutdown = False
        self._shutdown_lock = threading.Lock()
        atexit.register(self._python_exit)

    def init(self, address, timeout):
        """
        init async http client
        """
        ip, port = address.split(":")
        connection_nums = ConfigManager().connection_nums
        self.pool = ThreadPoolExecutor(max_workers=connection_nums, thread_name_prefix="AsyncHttpClient")
        self.timeout = timeout
        pool_class = HTTPConnectionPool
        kw = {}
        if ConfigManager().tls_config is not None:
            pool_class = HTTPSConnectionPool
            kw.update({
                'cert_reqs': 'CERT_REQUIRED',
                'ca_certs': ConfigManager().tls_config.root_cert_path,
                'cert_file': ConfigManager().tls_config.module_cert_path,
                'key_file': ConfigManager().tls_config.module_key_path,
                'assert_hostname': ConfigManager().tls_config.server_name
            })
            _logger.info(f"succeed to load tls config, on adder:{ip}:{port}")
        self.client = pool_class(host=ip, port=port, block=False, retries=3, maxsize=connection_nums, **kw)

    def submit_invoke_request(self, task: HttpTask, callback):
        """
        submit invoke request to frontend
        """
        with self._shutdown_lock:
            if self._shutdown:
                callback(None, RuntimeError("client is shutdown"))
                _logger.warning('cannot schedule new futures after shutdown')
                return

            self.pool.submit(self._call_request, task, callback)

    def shutdown(self):
        """
        clear pool
        """
        with self._shutdown_lock:
            self.client.close()
            self.pool.shutdown(wait=False)
            # clear threads for this ThreadPoolExecutor, avoid blocking process by join thread
            for k in self.pool._threads:
                thread._threads_queues.pop(k)
            self.pool._threads.clear()
            self._shutdown = True

    def _call_request(self, task: HttpTask, callback):
        """"
        Call Invoke Request.
        """
        _logger.debug(f"start to request: {task.task_id}")
        try:
            resp = self.client.request('POST', task.url, redirect=False,
                                       body=task.payload, headers=task.headers,
                                       timeout=urllib3.Timeout(connect=task.connect_timeout,
                                                               read=task.read_timeout),
                                       pool_timeout=_GETCON_TIMEOUT)
        except urllib3.exceptions.HTTPError as err:
            callback("", YRInvokeError(err, traceback.format_exc()))
            return
        _logger.debug(f"end to request: {task.task_id}")
        if resp.status != http.HTTPStatus.OK:
            if len(resp.data) == 0:
                callback(None, YRInvokeError(RuntimeError(
                    f"failed to send request, body is empty. code: {resp.status}"), traceback.format_exc()))
                return
        try:
            callback(resp.data)
        except Exception as err:
            callback(None, YRInvokeError(err, traceback.format_exc()))

    def _python_exit(self):
        self._shutdown = True
