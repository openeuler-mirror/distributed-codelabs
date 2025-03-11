#!/bin/bash
# Copyright (c) 2022 Huawei Technologies Co., Ltd.
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
#
# Edit this file to configure startup parameters, it is sourced to launch components.

# Address of worker and the value cannot be empty. (Default: "127.0.0.1:9088")
# AGENT_WORKER_ADDRESS="127.0.0.1:9088"

# Address of agent and the value cannot be empty. (Default: "127.0.0.1:9087")
# AGENT_ADDRESS="127.0.0.1:9087"

# Maximum time interval before a client is considered lost. (Default: "60")
# AGENT_CLIENT_TIMEOUT_S="60"

# Config rpc server thread number, must be greater than 0. (Default: "16")
# AGENT_RPC_THREAD_NUM="16"

# The directory where log files are stored. (Default: "~/.datasystem/logs")
# AGENT_LOG_DIR="~/.datasystem/logs"

# Prefix of log filename, default is program invocation short name. Use standard characters only. (Default: "")
# AGENT_LOG_FILENAME=""

# Async log buffer, unit is MB. (Default: "2")
# AGENT_LOG_ASYNC_BUFFER_MB="2"

# Maximum log file size (in MB), must be greater than 0. (Default: "400")
# AGENT_MAX_LOG_SIZE="400"

# All log files max size (in MB), must be greater than 400MB. Log will roll if the size is exceeded. (Default: "25")
# AGENT_MAX_LOG_FILE_NUM="25"

# If log_retention_day is greater than 0, any log file from your project whose last modified time is greater than log_retention_day days will be unlinked. If log_retention_day is equal 0, will not unlink log file by time. (Default: "0")
# AGENT_LOG_RETENTION_DAY="0"

# Flush log files with async mode. (Default: "false")
# AGENT_LOG_ASYNC="false"

# Compress old log files in .gz format. This parameter takes effect only when the size of the generated log is greater than max log size. (Default: "true")
# AGENT_LOG_COMPRESS="true"

# vlog level. (Default: "0")
# AGENT_V="0"

# Whether to enable the authentication function between components(agent, worker, master). (Default: "false")
# AGENT_ENABLE_COMPONENT_AUTH="false"

# Optimize the performance of the customer. Default server 5. The higher the throughput, the higher the value, but should be in range [1, 32]. (Default: "5")
# AGENT_ZMQ_SERVER_IO_CONTEXT="5"

# The directory to find ZMQ curve key files. This path must be specified when zmq authentication is enabled. (Default: "")
# AGENT_CURVE_KEY_DIR=""

# One of key component file path. Key components make up rootkey to encrypt or decrypt. (Default: "")
# AGENT_SECRET_KEY1=""

# One of key component file path. Key components make up rootkey to encrypt or decrypt. (Default: "")
# AGENT_SECRET_KEY2=""

# One of key component file path. Key components make up rootkey to encrypt or decrypt. (Default: "")
# AGENT_SECRET_KEY3=""

# One of key component file path. Key components make up rootkey to encrypt or decrypt. (Default: "")
# AGENT_SECRET_SALT=""
