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
CustomFilter class: custom filter
"""
import json
import logging
import logging.config
import os
import socket
import stat
import sys
import time

HOST_NAME = socket.gethostname()
POD_NAME = {'podname': HOST_NAME}


class CustomFilter(logging.Filterer):
    """
    CustomFilter custom filter
    """

    def filter(self, record):
        if len(record.msg) > MAX_ROW_SIZE:
            record.msg = record.msg[:MAX_ROW_SIZE]
        return True


# MAX_ROW_SIZE max row size of a log
MAX_ROW_SIZE = 1024
# python logging config
LOG_FORMAT = "[%(asctime)s %(levelname)s %(filename)s:%(lineno)d]" \
             " [%(podname)s %(thread)d] %(message)s"

# python log handler
RUNTIME_LOG = None


def setup_logging(runtime_id):
    """
    Set chmod of log file
    """
    # logging will collect processes information by default. Set these values
    # to False can improve performance
    logging.logProcesses = False
    logging.logMultiprocessing = False
    path = "/home/snuser/config/python-runtime-log.json"
    if os.getenv("YR_BARE_MENTAL") is not None:
        path = sys.path[0] + '/../config/python-runtime-log.json'
    with open(path, "r") as config_file:
        config = json.load(config_file)
        try:
            log_file_name = config["handlers"]["file"]["filename"]
        except BaseException as exp:
            log_file_name = "/home/snuser/log"
            print("log file name is invalid", exp)
        if runtime_id != "":
            os.path.join(log_file_name, runtime_id)
            log_file_name = os.path.join(log_file_name, runtime_id)
            os.makedirs(log_file_name, mode=0o755, exist_ok=True)
        os.environ["DATASYSTEM_CLIENT_LOG_DIR"] = log_file_name
        config["handlers"]["file"]["filename"] = os.path.join(log_file_name, "python-runtime.log")
        logging.config.dictConfig(config)
        os.chmod(config["handlers"]["file"]["filename"], stat.S_IWUSR | stat.S_IRUSR | stat.S_IRGRP)
    logging.Formatter.default_msec_format = "%s.%03d"
    logging.Formatter.converter = time.localtime


def init_log(runtime_id="", logger=None):
    """init log handler"""
    global RUNTIME_LOG
    if logger is not None:
        RUNTIME_LOG = logger
    else:
        setup_logging(runtime_id)
        RUNTIME_LOG = logging.getLogger("FileLogger")
        RUNTIME_LOG.addFilter(CustomFilter())
        RUNTIME_LOG = logging.LoggerAdapter(RUNTIME_LOG, POD_NAME)


def get_logger():
    """
    get_logger return runtime logger with basic config
    return: runtime_log
    """
    global RUNTIME_LOG
    if RUNTIME_LOG is None:
        init_log()
    return RUNTIME_LOG
