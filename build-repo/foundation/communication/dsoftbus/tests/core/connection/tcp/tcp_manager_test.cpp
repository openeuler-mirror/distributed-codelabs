/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <pthread.h>
#include <securec.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common_list.h"
#include "softbus_adapter_mem.h"
#include "softbus_base_listener.h"
#include "softbus_conn_interface.h"
#include "softbus_conn_manager.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_feature_config.h"
#include "softbus_log.h"
#include "softbus_tcp_connect_manager.h"
#include "softbus_tcp_socket.h"
#include "softbus_thread_pool.h"
#include "softbus_utils.h"

#define CLIENTPORT 6666
#define SERVERPORT 6667

static const int MAXLNE = 50;

using namespace testing::ext;

namespace OHOS {
const char *Ip = "127.0.0.1";
const char *g_data = "1234567890";

static uint32_t g_connectionId = 0;
static ConnectFuncInterface *g_interface = nullptr;
static ConnectResult g_result;
static ConnectCallback g_cb;
static int g_receivedDatalength = 0;
static int g_connServerInit = 0;

void TcpOnConnected(uint32_t connectionId, const ConnectionInfo *info)
{
    printf("TcpOnConnected %08x\n", connectionId);
}

void TcpOnDisConnect(uint32_t connectionId, const ConnectionInfo *info)
{
    printf("TcpOnDisConnect %08x\n", connectionId);
}

void TcpDataReceived(uint32_t connectionId, ConnModule moduleId, int64_t seq, char *data, int length)
{
    g_receivedDatalength = length;
    printf("nDataReceived with length:%d\n", length);
}

void TcpOnConnectionSuccessed(uint32_t requestId, uint32_t connectionId, const ConnectionInfo *info)
{
    g_connectionId = connectionId;
    printf("OnConnectionEnabled with requestId:%u connectionId:%08x\n", requestId, connectionId);
}

void TcpOnConnectionFailed(uint32_t requestId, int32_t reason)
{
    printf("OnConnectionFailed with requestId:%u reason:%d\n", requestId, reason);
}

class SoftbusTcpManagerTest : public testing::Test {
public:
    SoftbusTcpManagerTest()
    {}
    ~SoftbusTcpManagerTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SoftbusTcpManagerTest::SetUpTestCase(void)
{}

void SoftbusTcpManagerTest::TearDownTestCase(void)
{}

void SoftbusTcpManagerTest::SetUp(void)
{
    g_cb.OnConnected = TcpOnConnected;
    g_cb.OnDataReceived = TcpDataReceived;
    g_cb.OnDisconnected = TcpOnDisConnect;
    g_interface = ConnInitTcp(&g_cb);
    g_result.OnConnectSuccessed = TcpOnConnectionSuccessed;
    g_result.OnConnectFailed = TcpOnConnectionFailed;
    g_connectionId = 0;
    g_receivedDatalength = 0;
    g_connServerInit = ConnServerInit();
}

void SoftbusTcpManagerTest::TearDown(void)
{
    g_interface = nullptr;
    g_connServerInit = 0;
}

void CreateServer(void *arg)
{
    int listenfd, connfd, n;
    struct sockaddr_in servaddr;
    char buff[MAXLNE];
    unsigned int port = SERVERPORT;
    int defaultListen = 5;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return;
    }

    (void)memset_s(&servaddr, sizeof(servaddr), 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, Ip, &servaddr.sin_addr);
    servaddr.sin_port = htons(port);

    if (bind(listenfd, reinterpret_cast<struct sockaddr *>(&servaddr), sizeof(servaddr)) == -1) {
        close(listenfd);
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        return;
    }
    if (listen(listenfd, defaultListen) == -1) {
        close(listenfd);
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        return;
    }

    while (true) {
        if ((connfd = accept(listenfd, static_cast<struct sockaddr *>(nullptr), nullptr)) == -1) {
            printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
            continue;
        }
        break;
    }

    while (true) {
        n = recv(connfd, buff, MAXLNE, 0);
        if (n <= 0) {
            break;
        }
        printf("recv msg with length:%d from client\n", n);
        n = send(connfd, buff, static_cast<unsigned int>(n), 0);
        printf("send msg with length:%d to client\n", n);
    }
    close(connfd);
    close(listenfd);
}

/*
* @tc.name: testTcpManager001
* @tc.desc: test TcpGetConnNum
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager001, TestSize.Level1)
{
    EXPECT_EQ(0, TcpGetConnNum());
};

/*
* @tc.name: testTcpManager002
* @tc.desc: test TcpConnectDevice with invalid param
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager002, TestSize.Level1)
{
    int port = CLIENTPORT;
    uint32_t requestId = 1;
    ConnectOption option;
    option.type = CONNECT_BR;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    int ret;
    ret = TcpConnectDevice(nullptr, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
};

/*
* @tc.name: testTcpManager003
* @tc.desc: test TcpDisconnectDevice with wrong id
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager003, TestSize.Level1)
{
    EXPECT_EQ(SOFTBUS_ERR, TcpDisconnectDevice(g_connectionId));
};

/*
* @tc.name: testTcpManager004
* @tc.desc: test TcpGetConnectionInfo with invalid param
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager004, TestSize.Level1)
{
    ConnectionInfo info = {};
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpGetConnectionInfo(g_connectionId, nullptr));
    EXPECT_EQ(SOFTBUS_ERR, TcpGetConnectionInfo(g_connectionId, &info));
    EXPECT_EQ(false, info.isAvailable);
};

/*
* @tc.name: testTcpManager005
* @tc.desc: Test the BR and TCP start and stop listeners multiple times.
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager005, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_BR;
    info.socketOption.port = port;
    info.socketOption.moduleId = PROXY;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(SOFTBUS_ERR, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager006
* @tc.desc: test TcpDisconnectDevice
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager006, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option;
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpConnectDevice(&option, requestId, &g_result));
    sleep(1);
    EXPECT_EQ(2, TcpGetConnNum());
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(0, TcpGetConnNum());
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpManager007
* @tc.desc: test post out of max length
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager007, TestSize.Level1)
{
    pthread_t pid;

    int clientPort = CLIENTPORT;
    int serverPort = SERVERPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = clientPort;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = serverPort;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    option.socketOption.moduleId = PROXY;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    ConnPktHead head = {0};
    head.len = strlen(g_data);

    pthread_create(&pid, nullptr, (void *(*)(void *))CreateServer, nullptr);
    sleep(1);
    EXPECT_EQ(clientPort, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpConnectDevice(&option, requestId, &g_result));
    sleep(1);
    EXPECT_EQ(1, TcpGetConnNum());
    for (int i = 0; i < 3; i++) {
        char *data = (char *)SoftBusCalloc(sizeof(head) + head.len);
        if (data == nullptr) {
            continue;
        }
        (void)memcpy_s(data, sizeof(head), (void*)&head, sizeof(head));
        (void)memcpy_s(data + sizeof(head), (unsigned int)head.len, g_data, (unsigned int)head.len);
        EXPECT_EQ(SOFTBUS_OK, TcpPostBytes(g_connectionId, data, sizeof(ConnPktHead) + head.len, 0, 0));
        sleep(1);
        EXPECT_EQ(int(sizeof(ConnPktHead) + head.len), g_receivedDatalength);
        g_receivedDatalength = 0;
    }
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    EXPECT_EQ(0, TcpGetConnNum());
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(0, TcpGetConnNum());
    pthread_join(pid, nullptr);
}

/*
* @tc.name: testTcpManager008
* @tc.desc: test connect out of max connect num
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager008, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option;
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    int32_t maxConnNum;
    int32_t i = 0;
    if (SoftbusGetConfig(SOFTBUS_INT_CONN_TCP_MAX_CONN_NUM,
        (unsigned char*)&maxConnNum, sizeof(maxConnNum)) != SOFTBUS_OK) {
        LOG_ERR("get maxConnNum fail");
    }
    printf("maxConnNum: %d\n", maxConnNum);
    EXPECT_EQ(port, TcpStartListening(&info));
    while (i < maxConnNum) {
        EXPECT_EQ(SOFTBUS_OK, TcpConnectDevice(&option, requestId, &g_result));
        sleep(1);
        i += 2;
        EXPECT_EQ(i, TcpGetConnNum());
    }
    EXPECT_TRUE(SOFTBUS_OK != TcpConnectDevice(&option, requestId, &g_result));
    TcpDisconnectDeviceNow(&option);
    sleep(1);
    EXPECT_EQ(0, TcpGetConnNum());
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpManager009
* @tc.desc: test connect and post to self
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager009, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option;
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    option.socketOption.moduleId = PROXY;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    int maxDataLen;
    if (SoftbusGetConfig(SOFTBUS_INT_CONN_TCP_MAX_LENGTH,
        (unsigned char*)&maxDataLen, sizeof(maxDataLen)) != SOFTBUS_OK) {
        LOG_ERR("get maxDataLen fail");
    }
    printf("maxDataLen: %d\n", maxDataLen);
    ConnPktHead head = {0};
    head.len = maxDataLen + 1;

    char *data = (char *)SoftBusCalloc(sizeof(head) + head.len);
    if (data == nullptr) {
        printf("Failed to assign memory to data.");
        return;
    }
    (void)memcpy_s(data, sizeof(head), (void*)&head, sizeof(head));
    (void)memset_s(data + sizeof(head), (unsigned int)head.len, 0x1, (unsigned int)head.len);

    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpConnectDevice(&option, requestId, &g_result));
    sleep(1);
    EXPECT_EQ(2, TcpGetConnNum());
    EXPECT_EQ(SOFTBUS_OK, TcpPostBytes(g_connectionId, data, sizeof(ConnPktHead) + head.len, 0, 0));
    sleep(1);
    EXPECT_EQ(0, TcpGetConnNum());
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(0, TcpGetConnNum());
}

/*
* @tc.name: testTcpManager010
* @tc.desc: Test TcpConnectDevice with invalid -- option.type = connnet ble , moduleId = PROXY/AUTH/AUTH_P2P.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager010, TestSize.Level1)
{
    int port = CLIENTPORT;
    uint32_t requestId = 1;
    ConnectOption option;
    option.type = CONNECT_BLE;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    int ret;
    ret = TcpConnectDevice(nullptr, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    option.socketOption.moduleId = AUTH;
    ret = TcpConnectDevice(nullptr, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    option.socketOption.moduleId = AUTH_P2P;
    ret = TcpConnectDevice(nullptr, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
};

/*
* @tc.name: testTcpManager011
* @tc.desc: Test TcpConnectDevice with invalid -- option.type = connnet ble, \
\ moduleId = DIRECT_CHANNEL_SERVER_P2P/DIRECT_CHANNEL_CLIENT/DIRECT_CHANNEL_SERVER_WIFI.\
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager011, TestSize.Level1)
{
    int port = CLIENTPORT;
    uint32_t requestId = 1;
    ConnectOption option;
    option.type = CONNECT_BLE;
    option.socketOption.port = port;
    option.socketOption.moduleId = DIRECT_CHANNEL_SERVER_P2P;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    int ret;
    ret = TcpConnectDevice(nullptr, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    option.socketOption.moduleId = DIRECT_CHANNEL_CLIENT;
    ret = TcpConnectDevice(nullptr, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    option.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    ret = TcpConnectDevice(nullptr, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
};

/*
* @tc.name: testTcpManager012
* @tc.desc: Test TcpConnectDevice with invalid -- option.type = connnet p2p , moduleId = PROXY/AUTH/AUTH_P2P.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager012, TestSize.Level1)
{
    int port = CLIENTPORT;
    uint32_t requestId = 1;
    ConnectOption option;
    option.type = CONNECT_P2P;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    int ret;
    ret = TcpConnectDevice(nullptr, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    option.socketOption.moduleId = AUTH;
    ret = TcpConnectDevice(nullptr, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    option.socketOption.moduleId = AUTH_P2P;
    ret = TcpConnectDevice(nullptr, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
};

/*
* @tc.name: testTcpManager013
* @tc.desc: test TcpConnectDevice with invalid -- option.type = connnet p2p, \
\ moduleId = DIRECT_CHANNEL_SERVER_P2P/DIRECT_CHANNEL_CLIENT/DIRECT_CHANNEL_SERVER_WIFI.\
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager013, TestSize.Level1)
{
    int port = CLIENTPORT;
    uint32_t requestId = 1;
    ConnectOption option;
    option.type = CONNECT_P2P;
    option.socketOption.port = port;
    option.socketOption.moduleId = DIRECT_CHANNEL_SERVER_P2P;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    int ret;
    ret = TcpConnectDevice(nullptr, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    option.socketOption.moduleId = DIRECT_CHANNEL_CLIENT;
    ret = TcpConnectDevice(nullptr, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    option.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    ret = TcpConnectDevice(nullptr, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
};

/*
* @tc.name: testTcpManager014
* @tc.desc: Test start and stop listener multi times info.type = connect_br moduleId AUTH.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager014, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_BR;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager015
* @tc.desc: Test start and stop listener multi times info.type = connect_br  moduleId AUTH_P2P.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager015, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_BR;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_ERR, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager016
* @tc.desc: Test start and stop listener multi times info.type = connect_br moduleId DIRECT_CHANNEL_SERVER_P2P.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager016, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_BR;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager017
* @tc.desc: Test start and stop listener multi times info.type = connect_br  moduleId DIRECT_CHANNEL_CLIENT.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager017, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_BR;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_CLIENT;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager018
* @tc.desc: Test start and stop listener multi times info.type = connect_br  moduleId DIRECT_CHANNEL_SERVER_WIFI.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager018, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_BR;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager019
* @tc.desc: Test start and stop listener multi times  info.type = connect_ble  moduleId PROXY.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager019, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_BLE;
    info.socketOption.port = port;
    info.socketOption.moduleId = PROXY;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_ERR, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager020
* @tc.desc: Test start and stop listener multi times  info.type = connect_ble  moduleId AUTH.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager020, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_BLE;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager021
* @tc.desc: Test start and stop listener multi times  info.type = connect_ble  moduleId AUTH_P2P.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager021, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_BLE;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_ERR, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager022
* @tc.desc: Test GetTcpSockPort invalid fd.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The GetTcpSockPort operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager022, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);
    int fd = -1;
    int port = tcp->GetSockPort(fd);
    int ret = (port <= 0) ? SOFTBUS_ERR : SOFTBUS_OK;
    EXPECT_EQ(ret, SOFTBUS_ERR);

    LocalListenerInfo option = {
        .type = CONNECT_TCP,
        .socketOption = {
            .addr = "127.0.0.1",
            .port = CLIENTPORT,
            .moduleId = DIRECT_CHANNEL_SERVER_WIFI,
            .protocol = LNN_PROTOCOL_IP
        }
    };

    fd = tcp->OpenServerSocket(&option);
    ret = (fd <= 0) ? SOFTBUS_ERR : SOFTBUS_OK;
    ASSERT_TRUE(ret == SOFTBUS_OK);
    port = tcp->GetSockPort(fd);
    EXPECT_EQ(port, CLIENTPORT);
    ConnCloseSocket(fd);
};

/*
* @tc.name: testTcpManager023
* @tc.desc: Test SetIpTos invalid fd.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The SetIpTos operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager023, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);

    ConnectOption option = {
        .type = CONNECT_TCP,
        .socketOption = {
            .addr = {0},
            .port = CLIENTPORT,
            .moduleId = DIRECT_CHANNEL_SERVER_WIFI,
            .protocol = LNN_PROTOCOL_IP
        }
    };

    int fd =  tcp->OpenClientSocket(&option, "127.0.0.1", true);
    int tos = 1;
    int ret = SetIpTos(fd, tos);
    EXPECT_EQ(SOFTBUS_OK, ret);

    fd = -1;
    ret = SetIpTos(fd, tos);
    EXPECT_EQ(SOFTBUS_TCP_SOCKET_ERR, ret);
    ConnCloseSocket(fd);
};

/*
* @tc.name: testTcpManager024
* @tc.desc: Test ConnToggleNonBlockMode invalid fd.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The ConnToggleNonBlockMode operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager024, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);

    ConnectOption option = {
        .type = CONNECT_TCP,
        .socketOption = {
            .addr = {0},
            .port = CLIENTPORT,
            .moduleId = DIRECT_CHANNEL_SERVER_WIFI,
            .protocol = LNN_PROTOCOL_IP
        }
    };
    int fd = -1;
    bool isNonBlock = true;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ConnToggleNonBlockMode(fd, isNonBlock));

    fd =  tcp->OpenClientSocket(&option, "127.0.0.1", true);
    EXPECT_EQ(SOFTBUS_OK, ConnToggleNonBlockMode(fd, isNonBlock));
    ConnCloseSocket(fd);
};

/*
* @tc.name: testTcpManager025
* @tc.desc: Test ConnSendSocketData invalid fd.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The ConnSendSocketData operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager025, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);

    ConnectOption option = {
        .type = CONNECT_TCP,
        .socketOption = {
            .addr = {0},
            .port = CLIENTPORT,
            .moduleId = DIRECT_CHANNEL_SERVER_WIFI,
            .protocol = LNN_PROTOCOL_IP
        }
    };

    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    int clientfd = tcp->OpenClientSocket(&option, "127.0.0.1", false);
    ssize_t bytes = ConnSendSocketData(clientfd, "Hello world", 11, 0);
    EXPECT_EQ(bytes, -1);
    ConnShutdownSocket(clientfd);

    clientfd = tcp->OpenClientSocket(&option, "127.0.0.1", true);
    bytes = ConnSendSocketData(clientfd, "Hello world", 11, 0);
    EXPECT_EQ(bytes, -1);
    ConnShutdownSocket(clientfd);
};

/*
* @tc.name: testTcpManager026
* @tc.desc: Test ConnSendSocketData invalid buf len.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The OpenTcpClientSocket and ConnSendSocketData operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager026, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);

    ConnectOption option = {
        .type = CONNECT_TCP,
        .socketOption = {
            .addr = {0},
            .port = CLIENTPORT,
            .moduleId = DIRECT_CHANNEL_SERVER_WIFI,
            .protocol = LNN_PROTOCOL_IP
        }
    };

    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    int clientfd = tcp->OpenClientSocket(&option, "127.0.0.1", true);
    int ret = (clientfd <= 0) ? SOFTBUS_ERR : SOFTBUS_OK;
    EXPECT_EQ(ret, SOFTBUS_OK);

    int bytes = ConnSendSocketData(clientfd, nullptr, 10, 0);
    EXPECT_EQ(bytes, -1);

    bytes = ConnSendSocketData(clientfd, "hello world!", 0, 0);
    EXPECT_EQ(bytes, -1);

    bytes = ConnSendSocketData(clientfd, "hello world!", 12, 0);
    EXPECT_EQ(bytes, -1);
    ConnShutdownSocket(clientfd);
};

/*
* @tc.name: testTcpManager027
* @tc.desc: Test ConnGetSocketError invalid param.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The ConnGetSocketError and operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager027, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);

    ConnectOption option = {
        .type = CONNECT_TCP,
        .socketOption = {
            .addr = {0},
            .port = CLIENTPORT,
            .moduleId = DIRECT_CHANNEL_SERVER_WIFI,
            .protocol = LNN_PROTOCOL_IP
        }
    };

    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    int clientfd = tcp->OpenClientSocket(&option, "127.0.0.1", true);
    int ret = (clientfd <= 0) ? SOFTBUS_ERR : SOFTBUS_OK;
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_TRUE(ConnGetSocketError(clientfd) != 0);
    ConnCloseSocket(clientfd);
};

/*
* @tc.name: testTcpManager028
* @tc.desc: Test the BLE and TCP start and stop listeners multiple times.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager028, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_BLE;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager029
* @tc.desc: Test BLE and TCP start and stop listeners under DIRECT_CHANNEL_CLIENT multiple times.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager029, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_BLE;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_CLIENT;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager030
* @tc.desc: Test BLE and TCP start and stop listeners under DIRECT_CHANNEL_SERVER_WIFI multiple times.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager030, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_BLE;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager031
* @tc.desc: Test P2P and TCP start and stop listeners under PROXY multiple times.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager031, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_P2P;
    info.socketOption.port = port;
    info.socketOption.moduleId = PROXY;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_ERR, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager032
* @tc.desc: Test P2P and TCP start and stop listeners under AUTH multiple times.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager032, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_P2P;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager033
* @tc.desc: Test P2P and TCP start and stop listeners under AUTH_P2P multiple times.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager033, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_P2P;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_ERR, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager034
* @tc.desc: Test P2P and TCP start and stop listeners under DIRECT_CHANNEL_SERVER_P2P multiple times.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager034, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_P2P;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager035
* @tc.desc: Test P2P and TCP start and stop listeners under DIRECT_CHANNEL_CLIENT multiple times.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager035, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_P2P;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_CLIENT;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager036
* @tc.desc: Test P2P and TCP start and stop listeners under DIRECT_CHANNEL_SERVER_WIFI multiple times.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpStartListening and TcpStopListening operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager036, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_P2P;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStartListening(nullptr));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, TcpStopListening(nullptr));

    info.type = CONNECT_TCP;
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
    EXPECT_TRUE(SOFTBUS_OK == TcpStopListening(&info));
};

/*
* @tc.name: testTcpManager037
* @tc.desc: Test SetIpTos return yes or no.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The SetIpTos operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager037, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);

    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int fd = tcp->OpenServerSocket(&info);
    uint32_t tos = 65535;
    int rc = SetIpTos(fd, tos);
    EXPECT_EQ(rc, SOFTBUS_OK);
};

/*
* @tc.name: testTcpManager038
* @tc.desc: Test OpenTcpServerSocket Open succeed yes or no.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The OpenTcpServerSocket operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager038, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);

    int port = SERVERPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_TRUE(tcp->OpenServerSocket(&info) > 0);
}

/*
* @tc.name: testTcpManager039
* @tc.desc: Test OpenTcpClientSocket Open succeed yes or no.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The SetIpTos and OpenTcpClientSocket operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager039, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);

    LocalListenerInfo info = {
        .type = CONNECT_TCP,
        .socketOption = {
            .addr = {0},
            .port = CLIENTPORT,
            .moduleId = DIRECT_CHANNEL_SERVER_WIFI,
            .protocol = LNN_PROTOCOL_IP
        }
    };
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    int fd = tcp->OpenServerSocket(&info);
    EXPECT_EQ(SetIpTos(fd, 65535), SOFTBUS_OK);

    ConnectOption option = {
        .type = CONNECT_TCP,
        .socketOption = {
            .addr = {0},
            .port = CLIENTPORT,
            .moduleId = DIRECT_CHANNEL_SERVER_WIFI,
            .protocol = LNN_PROTOCOL_IP
        }
    };
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    EXPECT_TRUE(tcp->OpenClientSocket(&option, Ip, true) > 0);
};

/*
* @tc.name: testTcpManager040
* @tc.desc: Test ConnToggleNonBlockMode param is invalid yes or no.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The SetIpTos and ConnToggleNonBlockMode operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager040, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);

    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int fd = tcp->OpenServerSocket(&info);
    EXPECT_TRUE(fd > 0);
    EXPECT_EQ(SetIpTos(fd, 65535), SOFTBUS_OK);
    EXPECT_TRUE(ConnToggleNonBlockMode(fd, true) == 0);
};

/*
* @tc.name: testTcpManager041
* @tc.desc: Test GetTcpSockPort param is invalid yes or no.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The GetTcpSockPort operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager041, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);

    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int fd = tcp->OpenServerSocket(&info);
    EXPECT_TRUE(tcp->GetSockPort(fd) > 0);
};

/*
* @tc.name: testTcpManager042
* @tc.desc: test ConnSendSocketData SendData successful yes or no.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The ConnSendSocketData operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager042, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);

    ConnectOption option = {
        .type = CONNECT_TCP,
        .socketOption = {
            .addr = "",
            .port = CLIENTPORT,
            .moduleId = DIRECT_CHANNEL_SERVER_WIFI,
            .protocol = LNN_PROTOCOL_IP
        }
    };

    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    int fd = tcp->OpenClientSocket(&option, option.socketOption.addr, false);
    const char * buf = "SendDataTest";
    EXPECT_EQ(ConnSendSocketData(fd, buf, 13, 0), -1);
};

/*
* @tc.name: testTcpManager043
* @tc.desc: Test whether the CloseTcpFd function runs successfully.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The OpenTcpClientSocket operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager043, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);

    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int fd = tcp->OpenServerSocket(&info);
    ConnCloseSocket(fd);
    EXPECT_TRUE(fd >= 0);
};

/*
* @tc.name: testTcpManager044
* @tc.desc: Test ConnSendSocketData param is invalid yes or no.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Nonzero
* @tc.type: FUNC
* @tc.require: The ConnSendSocketData operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager044, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);

    ConnectOption option = {
        .type = CONNECT_TCP,
        .socketOption = {
            .addr = {0},
            .port = CLIENTPORT,
            .moduleId = DIRECT_CHANNEL_SERVER_WIFI,
            .protocol = LNN_PROTOCOL_IP
        }
    };

    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    int fd = tcp->OpenClientSocket(&option, option.socketOption.addr, false);
    const char * buf = "SendDataTest";
    EXPECT_EQ(ConnSendSocketData(fd, buf, 13, 0xffff), -1);
};

/*
* @tc.name: testTcpManager045
* @tc.desc: Test ConnCloseSocket function successful yes or no.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The ConnShutdownSocket and OpenTcpClientSocket operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager045, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);

    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int fd = tcp->OpenServerSocket(&info);
    ConnShutdownSocket(fd);
    EXPECT_TRUE(fd >= 0);
};

/*
* @tc.name: testTcpManager046
* @tc.desc: Test ConnSetTcpKeepAlive fd param invalid.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Nonzero
* @tc.type: FUNC
* @tc.require: The ConnSetTcpKeepAlive operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager046, TestSize.Level1)
{
    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    EXPECT_EQ(ConnSetTcpKeepAlive(-100, 65535), -1);
};

/*
* @tc.name: testTcpManager047
* @tc.desc: Test ConnSetTcpKeepAlive second param invalid.
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The ConnSetTcpKeepAlive operates normally.
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpManager047, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);

    int port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int fd = tcp->OpenServerSocket(&info);
    EXPECT_EQ(ConnSetTcpKeepAlive(fd, 100), SOFTBUS_OK);
};

/*
* @tc.name: testTcpDisconnectDeviceNow001
* @tc.desc: test TcpDisconnectDeviceNow invaild parma
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(SoftbusTcpManagerTest, testTcpDisconnectDeviceNow001, TestSize.Level1)
{
    int ret = TcpDisconnectDeviceNow(nullptr);
    EXPECT_EQ(ret, SOFTBUS_ERR);
}
}