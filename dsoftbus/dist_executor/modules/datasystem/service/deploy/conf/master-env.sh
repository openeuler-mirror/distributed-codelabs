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

# Address of master and the value cannot be empty. (Default: "127.0.0.1:9089")
# MASTER_ADDRESS="127.0.0.1:9089"

# Config MASTER back store directory and must specify in rocksdb scenario. The rocksdb database is used to persistently store the metadata stored in the master so that the metadata before the restart can be re-obtained when the master restarts. (Default: "~/.datasystem/rocksdb")
# MASTER_BACKEND_STORE_DIR="~/.datasystem/rocksdb"

# Controls whether rocksdb sets sync to true when writing data. (Default: "false")
# MASTER_ROCKSDB_SYNC_WRITE="false"

# The redis IP address host:port. If the redis service is required, the address must be set and match the worker's redis address. ex: 127.0.0.1:6379. (Default: "")
# MASTER_REDIS_ADDRESS=""

# Maximum time interval before a node is considered lost. (Default: "60")
# MASTER_NODE_TIMEOUT_S="60"

# Maximum time interval for the master to determine node death. (Default: "7200")
# MASTER_NODE_DEAD_TIMEOUT_S="7200"

# Interval in milliseconds at which master check heartbeat status. (Default: "30000")
# MASTER_CHECK_HEARTBEAT_INTERVAL_MS="30000"

# File path of trusted CA/ca bundle file, optional. Use standard characters only. (Default: "")
# MASTER_REDIS_CA=""

# File path of client certificate file, optional. Use standard characters only. (Default: "")
# MASTER_REDIS_CERT=""

# File path of client private key, optional. Use standard characters only. (Default: "")
# MASTER_REDIS_KEY=""

# The redis username for auth. (Default: "")
# MASTER_REDIS_USERNAME=""

# The redis password for auth. (Default: "")
# MASTER_REDIS_PASSWD=""

# Config rpc server thread number, must be greater than 0. (Default: "16")
# MASTER_RPC_THREAD_NUM="16"

# The directory where log files are stored. (Default: "~/.datasystem/logs")
# MASTER_LOG_DIR="~/.datasystem/logs"

# Prefix of log filename, default is program invocation short name. Use standard characters only. (Default: "")
# MASTER_LOG_FILENAME=""

# Async log buffer, unit is MB. (Default: "2")
# MASTER_LOG_ASYNC_BUFFER_MB="2"

# Maximum log file size (in MB), must be greater than 0. (Default: "400")
# MASTER_MAX_LOG_SIZE="400"

# All log files max size (in MB), must be greater than 400MB. Log will roll if the size is exceeded. (Default: "25")
# MASTER_MAX_LOG_FILE_NUM="25"

# If log_retention_day is greater than 0, any log file from your project whose last modified time is greater than log_retention_day days will be unlinked. If log_retention_day is equal 0, will not unlink log file by time. (Default: "0")
# MASTER_LOG_RETENTION_DAY="0"

# Flush log files with async mode. (Default: "false")
# MASTER_LOG_ASYNC="false"

# Compress old log files in .gz format. This parameter takes effect only when the size of the generated log is greater than max log size. (Default: "true")
# MASTER_LOG_COMPRESS="true"

# vlog level. (Default: "0")
# MASTER_V="0"

# Whether to enable the authentication function between components(agent, worker, master). (Default: "false")
# MASTER_ENABLE_COMPONENT_AUTH="false"

# Optimize the performance of the customer. Default server 5. The higher the throughput, the higher the value, but should be in range [1, 32]. (Default: "5")
# MASTER_ZMQ_SERVER_IO_CONTEXT="5"

# The directory to find ZMQ curve key files. This path must be specified when zmq authentication is enabled. (Default: "")
# MASTER_CURVE_KEY_DIR=""

# One of key component file path. Key components make up rootkey to encrypt or decrypt. (Default: "")
# MASTER_SECRET_KEY1=""

# One of key component file path. Key components make up rootkey to encrypt or decrypt. (Default: "")
# MASTER_SECRET_KEY2=""

# One of key component file path. Key components make up rootkey to encrypt or decrypt. (Default: "")
# MASTER_SECRET_KEY3=""

# One of key component file path. Key components make up rootkey to encrypt or decrypt. (Default: "")
# MASTER_SECRET_SALT=""
