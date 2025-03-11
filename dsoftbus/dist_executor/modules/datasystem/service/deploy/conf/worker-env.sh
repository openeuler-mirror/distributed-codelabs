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

# Address of worker and the value cannot be empty. Multiple nodes can be configured, such as ("127.0.0.1:18482" "127.0.0.2:18482"). (Default: "127.0.0.1:9088")
# WORKER_ADDRESS="127.0.0.1:9088"

# Address of master and the value cannot be empty. (Default: "127.0.0.1:9089")
# WORKER_MASTER_ADDRESS="127.0.0.1:9089"

# Indicates whether the worker allows using the shared memory. (Default: "true")
# WORKER_ENABLE_SHARED_MEMORY="true"

# Upper limit of the shared memory, the unit is mb, must be greater than 0. (Default: "1024")
# WORKER_SHARED_MEMORY_SIZE_MB="1024"

# Time interval between worker and master heartbeats. (Default: "1000")
# WORKER_HEARTBEAT_INTERVAL_MS="1000"

# Indicates whether to enable the tenant authentication, default is false. (Default: "false")
# WORKER_AUTHORIZATION_ENABLE="false"

# Enable unix domain socket. (Default: "true")
# WORKER_ENABLE_UDS="true"

# The directory to store unix domain socket file. The UDS generates temporary files in this path. (Default: "~/.datasystem/unix_domain_socket_dir")
# WORKER_UNIX_DOMAIN_SOCKET_DIR="~/.datasystem/unix_domain_socket_dir"

# The number of regular backend socket for stream cache. (Default: "32")
# WORKER_SC_REGULAR_SOCKET_NUM="32"

# The number of stream backend socket for stream cache. (Default: "32")
# WORKER_SC_STREAM_SOCKET_NUM="32"

# The number of worker service for object cache. (Default: "32")
# WORKER_OC_THREAD_NUM="32"

# Thread number of eviction for object cache. (Default: "1")
# WORKER_EVICTION_THREAD_NUM="1"

# Client reconnect wait seconds, default is 5. (Default: "5")
# WORKER_CLIENT_RECONNECT_WAIT_S="5"

# Size of the page used for caching worker files. The valid range is 4096-1073741824. (Default: "1048576")
# WORKER_PAGE_SIZE="1048576"

# The num of threads used to send elements to remote worker. (Default: "8")
# WORKER_REMOTE_SEND_THREAD_NUM="8"

# The redis IP address host:port. If the redis service is required, the address must be set. ex: 127.0.0.1:6379. (Default: "")
# WORKER_REDIS_ADDRESS=""

# The path and file name prefix of the spilling, empty means spill disabled. (Default: "")
# WORKER_SPILL_DIRECTORY=""

# The size limit of spilled data, 0 means unlimited. (Default: "0")
# WORKER_SPILL_SIZE_LIMIT="0"

# The redis username for auth. (Default: "")
# WORKER_REDIS_USERNAME=""

# The redis password for auth. (Default: "")
# WORKER_REDIS_PASSWD=""

# File path of trusted CA/ca bundle file, optional. Use standard characters only. (Default: "")
# WORKER_REDIS_CA=""

# File path of client certificate file, optional. Use standard characters only. (Default: "")
# WORKER_REDIS_CERT=""

# File path of client private key, optional. Use standard characters only. (Default: "")
# WORKER_REDIS_KEY=""

# Maximum number of clients that can be connected to a worker. Value range: [1, 10000]. (Default: "200")
# WORKER_MAX_CLIENT_NUM="200"

# Config rpc server thread number, must be greater than 0. (Default: "16")
# WORKER_RPC_THREAD_NUM="16"

# The directory where log files are stored. (Default: "~/.datasystem/logs")
# WORKER_LOG_DIR="~/.datasystem/logs"

# Prefix of log filename, default is program invocation short name. Use standard characters only. (Default: "")
# WORKER_LOG_FILENAME=""

# Async log buffer, unit is MB. (Default: "2")
# WORKER_LOG_ASYNC_BUFFER_MB="2"

# Maximum log file size (in MB), must be greater than 0. (Default: "400")
# WORKER_MAX_LOG_SIZE="400"

# All log files max size (in MB), must be greater than 400MB. Log will roll if the size is exceeded. (Default: "25")
# WORKER_MAX_LOG_FILE_NUM="25"

# If log_retention_day is greater than 0, any log file from your project whose last modified time is greater than log_retention_day days will be unlinked. If log_retention_day is equal 0, will not unlink log file by time. (Default: "0")
# WORKER_LOG_RETENTION_DAY="0"

# Flush log files with async mode. (Default: "false")
# WORKER_LOG_ASYNC="false"

# Compress old log files in .gz format. This parameter takes effect only when the size of the generated log is greater than max log size. (Default: "true")
# WORKER_LOG_COMPRESS="true"

# vlog level. (Default: "0")
# WORKER_V="0"

# Whether to enable the authentication function between components(agent, worker, master). (Default: "false")
# WORKER_ENABLE_COMPONENT_AUTH="false"

# Optimize the performance of the customer. Default server 5. The higher the throughput, the higher the value, but should be in range [1, 32]. (Default: "5")
# WORKER_ZMQ_SERVER_IO_CONTEXT="5"

# The directory to find ZMQ curve key files. This path must be specified when zmq authentication is enabled. (Default: "")
# WORKER_CURVE_KEY_DIR=""

# One of key component file path. Key components make up rootkey to encrypt or decrypt. (Default: "")
# WORKER_SECRET_KEY1=""

# One of key component file path. Key components make up rootkey to encrypt or decrypt. (Default: "")
# WORKER_SECRET_KEY2=""

# One of key component file path. Key components make up rootkey to encrypt or decrypt. (Default: "")
# WORKER_SECRET_KEY3=""

# One of key component file path. Key components make up rootkey to encrypt or decrypt. (Default: "")
# WORKER_SECRET_SALT=""
