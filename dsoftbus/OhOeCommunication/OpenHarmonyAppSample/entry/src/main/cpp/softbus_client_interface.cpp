#include <cstdio>
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include "napi/native_api.h"
#include "hilog/log.h"
#include "softbus_client_interface.h"
#include "discovery_service.h"
#include "softbus_bus_center.h"
#include "session.h"
#include "softbus_common.h"

using namespace std;

#define PACKAGE_NAME "softbus_sample"
#define LOCAL_SESSION_NAME "session_test"
#define DEFAULT_PUBLISH_ID 123
#define DEFAULT_CAPABILITY "osdCapability"
#define DEFAULT_SESSION_GROUP "group_test"

const int DISCOVER_SLEEP_TIME = 1000; // 发现设备的等待时间，确保设备信息获取到

static callBack *callBackFunc; // 接收消息后传给APP侧的回调函数
int g_sessionId; 
std::map<std::string, int> sessionNameAndId; // session name和session id的对应map
std::map<std::string, DeviceDetail> deviceIdAndInfo; 

string getCurrentTime();

/**
 * @brief 服务发布成功后的回调函数
 * 
 * @param publishId 服务id
 */
void PublishSuccess(int publishId) {
    OH_LOG_INFO(LOG_APP, "[INFO] <PublishSuccess>CB: publish %{public}d done", publishId);
}

/**
 * @brief 服务发布失败后的回调函数
 * 
 * @param publishId 服务ID
 * @param reason 失败原因
 */
void PublishFailed(int publishId, PublishFailReason reason) {
    OH_LOG_ERROR(LOG_APP, "[ERROR] <PublishFailed>CB: publish %{public}d failed, reason=%{public}d", publishId, (int)reason);
}

/**
 * 连接通道建立后的接收到消息的回调函数
 * @param sessionId 建立连接的设备的sessionId，必须是<sessionIdName>中数据。
 * @param data 对端设备发送的数据
 * @param dataLength 对端设备发送数据的大小
 */
void OnReceiveCallback(int sessionId, const void *data, unsigned int dataLength) {
    OH_LOG_INFO(LOG_APP, "[INFO] <OnReceiveCallback> The sessionId: %{public}d data is received and the callback function is started.", sessionId);
    callBackFunc((char *)data);
}

/**
 * @brief 服务发布的封装接口
 * 
 * @return int 发布成功返回0，否则返回1
 */
int PublishServiceInterface() {
    OH_LOG_INFO(LOG_APP, "[INFO] Start to PublishService");
    PublishInfo info = {
        .publishId = DEFAULT_PUBLISH_ID,
        .mode = DISCOVER_MODE_PASSIVE,
        .medium = COAP,
        .freq = LOW,
        .capability = DEFAULT_CAPABILITY,
        .capabilityData = nullptr,
        .dataLen = 0,
    };
    IPublishCallback cb = {
        .OnPublishSuccess = PublishSuccess,
        .OnPublishFail = PublishFailed,
    };

    int hwpadPublicResult = PublishService(PACKAGE_NAME, &info, &cb);
    return hwpadPublicResult;
}

/**
 * @brief 停止发布服务封装接口
 * 
 */
void UnPublishServiceInterface() {
    int ret;
    ret = UnPublishService(PACKAGE_NAME, DEFAULT_PUBLISH_ID);
    if (ret != 0) {
        OH_LOG_ERROR(LOG_APP, "[ERROR] UnPublishService hwpad fail:%{public}d", ret);
    }
}

/**
 * @brief 组网内设备探测到的回调函数
 * 
 * @param device 设备信息
 */
void DeviceFound(const DeviceInfo *device) {
    unsigned int i;
    OH_LOG_INFO(LOG_APP, "[INFO] <DeviceFound>CB: Device has found:");
    OH_LOG_INFO(LOG_APP, "\tdevId=%{public}s", device->devId);
    OH_LOG_INFO(LOG_APP, "\tdevName=%{public}s", device->devName);
    OH_LOG_INFO(LOG_APP, "\tdevType=%{public}d", device->devType);
    OH_LOG_INFO(LOG_APP, "\taddrNum=%{public}d", device->addrNum);
    for (i = 0; i < device->addrNum; i++) {
        OH_LOG_INFO(LOG_APP, "\taddr%{public}d:type=%{public}d,", i + 1, device->addr[i].type);
        switch (device->addr[i].type) {
        case CONNECTION_ADDR_WLAN:
        case CONNECTION_ADDR_ETH:
            OH_LOG_INFO(LOG_APP, "\tip=%{public}s,port=%{public}d,", device->addr[i].info.ip.ip, device->addr[i].info.ip.port);
            break;
        default:
            break;
        }
        OH_LOG_INFO(LOG_APP, "\tpeerUid=%{public}s", device->addr[i].peerUid);
    }
    OH_LOG_INFO(LOG_APP, "\tcapabilityBitmapNum=%{public}d", device->capabilityBitmapNum);
    for (i = 0; i < device->addrNum; i++) {
        OH_LOG_INFO(LOG_APP, "\tcapabilityBitmap[%{public}d]=0x%x", i + 1, device->capabilityBitmap[i]);
    }
    OH_LOG_INFO(LOG_APP, "\tcustData=%{public}s", device->custData);

    DeviceDetail detail;
    detail.deviceName = device->devName;
    detail.deviceId = device->devId;
    detail.ipaddress = device->addr[0].info.ip.ip;
    detail.port = device->addr[0].info.ip.port;
    deviceIdAndInfo[device->devId] = detail;
}

/**
 * @brief 探测组网内设备成功的回调函数
 * 
 * @param subscribeId 其他设备发布服务的ID
 */
void DiscoverySuccess(int subscribeId) {
    OH_LOG_INFO(LOG_APP, "[INFO] <DiscoverySuccess>CB: discover subscribeId=%{public}d", subscribeId);
}

/**
 * @brief 探测组网内设备失败的回调函数
 * 
 * @param subscribeId 其他设备发布服务的ID
 * @param reason 失败原因
 */
void DiscoveryFailed(int subscribeId, DiscoveryFailReason reason) {
    OH_LOG_ERROR(LOG_APP, "[ERROR] <DiscoveryFailed>CB: discover subscribeId=%{public}d failed, reason=%{public}d", subscribeId, (int)reason);
}

/**
 * @brief 开启组网内设备探测的封装函数
 * 
 * @return int 成功返回0，失败返回1
 */
int DiscoveryInterface() {
    OH_LOG_INFO(LOG_APP, "[INFO] Start to Discovery");
    SubscribeInfo info = {
        .subscribeId = DEFAULT_PUBLISH_ID,
        .mode = DISCOVER_MODE_ACTIVE,
        .medium = COAP,
        .freq = LOW,
        .isSameAccount = false,
        .isWakeRemote = false,
        .capability = DEFAULT_CAPABILITY,
        .capabilityData = nullptr,
        .dataLen = 0,
    };
    IDiscoveryCallback cb = {
        .OnDeviceFound = DeviceFound,
        .OnDiscoverFailed = DiscoveryFailed,
        .OnDiscoverySuccess = DiscoverySuccess,
    };

    deviceIdAndInfo.clear();
    int hwpadDiscoveryResult = StartDiscovery(PACKAGE_NAME, &info, &cb);
    sleep(DISCOVER_SLEEP_TIME);
    return hwpadDiscoveryResult;
}

/**
 * @brief 停止其他设备探测的封装函数
 * 
 */
void StopDiscoveryInterface() {
    int ret;
    ret = StopDiscovery(PACKAGE_NAME, DEFAULT_PUBLISH_ID);
    if (ret) {
        OH_LOG_ERROR(LOG_APP, "[ERROR] StopDiscovery fail:%{public}d", ret);
    }
}

/**
 * @brief session通道建立的回调函数
 * 
 * @param sessionId 建立的session的id
 * @param result session建立结果
 * @return int 成功返回0，失败返回1
 */
int SessionOpened(int sessionId, int result) {
    if (result == 0) {
        g_sessionId = sessionId;
        char peerSessionName[50];
        GetPeerSessionName(sessionId, peerSessionName, 50);
        sessionNameAndId[peerSessionName] = sessionId;
        OH_LOG_INFO(LOG_APP, "[INFO] <SessionOpened>CB: session %{public}d open success, session name is: %{public}s", sessionId, peerSessionName);
    } else {
        OH_LOG_ERROR(LOG_APP, "[ERROR] <SessionOpened>CB: session %{public}d open failed, result is: %{public}d", sessionId, result);
    }

    return result;
}

/**
 * @brief session关闭
 * 
 * @param sessionId 要关闭的session的Id
 */
void SessionClosed(int sessionId) {
    OH_LOG_INFO(LOG_APP, "[INFO] <SessionClosed>CB: session %{public}d closed", sessionId);
}

/**
 * @brief 其他设备通过sendMessage方法发送消息，接收数据后的回调函数
 * 
 * @param sessionId 和对端设备的session id
 * @param data 接收到的数据
 * @param dataLen 接收到的数据长度
 */
void MessageReceived(int sessionId, const void *data, unsigned int dataLen) {
    OH_LOG_INFO(LOG_APP, "[INFO] <MessageReceived>CB: session %{public}d received %u bytes message=%{public}s", sessionId, dataLen, (const char *)data);
}

/**
 * @brief 创建session服务的封装接口，需要指定session打开后和接受消息后的回调函数
 * 
 * @param func 自定义接受消息后的回调函数
 * @return int 成功返回0，否则返回1
 */
int CreateSessionServerInterface(callBack *func) {
    OH_LOG_INFO(LOG_APP, "[INFO] Start to CreateSessionServer");
    callBackFunc = func;
    const ISessionListener sessionCB = {
        .OnSessionOpened = SessionOpened,
        .OnSessionClosed = SessionClosed,
        .OnBytesReceived = OnReceiveCallback,
        .OnMessageReceived = MessageReceived,
    };
    int hwpadSessionResult = CreateSessionServer(PACKAGE_NAME, LOCAL_SESSION_NAME, &sessionCB);

    return hwpadSessionResult;
}

/**
 * @brief 停止创建session的服务
 * 
 */
void RemoveSessionServerInterface() {
    int ret;
    ret = RemoveSessionServer(PACKAGE_NAME, LOCAL_SESSION_NAME);
    if (ret) {
        OH_LOG_ERROR(LOG_APP, "[ERROR] RemoveSessionServer hwpad fail:%{public}d", ret);
    }
}

/**
 * @brief 创建session的封装接口
 * 
 * @param peerNetworkId 对端设备的network id
 * @param peerSessionName 对端设备的session名称
 * @return int 成功返回0，否则返回1
 */
int OpenSessionInterface(const char *peerNetworkId, const char *peerSessionName) {
    OH_LOG_INFO(LOG_APP, "[INFO] Start to open session peerNetworkId: %{public}s peerSessionName: %{public}s", peerNetworkId, peerSessionName);
    int timeout = 5;
    g_sessionId = -1;

    SessionAttribute attr = {
        .dataType = TYPE_BYTES,
        .linkTypeNum = 1,
        .linkType = {LINK_TYPE_WIFI_WLAN_2G},
        .attr = {RAW_STREAM},
    };

    int sessionId = OpenSession(LOCAL_SESSION_NAME, peerSessionName, peerNetworkId,
                                DEFAULT_SESSION_GROUP, &attr);
    if (sessionId < 0) {
        OH_LOG_ERROR(LOG_APP, "[ERROR] OpenSessionInterface fail, ret=%{public}d", sessionId);
        return sessionId;
    }
    // 等待5s确保session id获取到
    while (timeout) {
        if (g_sessionId == sessionId) {
            OH_LOG_INFO(LOG_APP, "[INFO] Obtaining the sessionId of device (sessionName:%{public}s, networkId: %{public}s), sessionId is: %{public}d",
                        peerSessionName, peerNetworkId, sessionId);
            return sessionId;
        }
        timeout--;
        sleep(1);
    }
    return -1;
}

/**
 * @brief 关闭session的封装接口
 * 
 * @param sessionId 要关闭的session id
 */
void CloseSessionInterface(int sessionId) {
    CloseSession(sessionId);
}

/**
 * @brief 获得组网内已认证的设备信息
 * 
 * @param dev 接收设备信息的指针
 * @return int 成功返回设备数量，失败返回1
 */
int GetAllNodeDeviceInfoInterface(NodeBasicInfo **dev) {
    int ret, num;

    ret = GetAllNodeDeviceInfo(PACKAGE_NAME, dev, &num);
    if (ret) {
        OH_LOG_ERROR(LOG_APP, "[ERROR] GetAllNodeDeviceInfo by %{public}s fail:%{public}d", PACKAGE_NAME, ret);
        return -1;
    }

    OH_LOG_INFO(LOG_APP, "[INFO] <GetAllNodeDeviceInfo> by %{public}s return %{public}d Node", PACKAGE_NAME, num);
    return num;
}

/**
 * @brief 丰富通过GetAllNodeDeviceInfo接口获取的设备信息
 * 
 * @param deviceDetail 接收设备详细信息的指针
 * @param dev 通过GetAllNodeDeviceInfo获取的设备接口
 * @param devNum 发现的设备的数量
 * @return int 获取到信息的设备的数量
 */
int GetDeviceDetailInterface(DeviceDetail *deviceDetail, NodeBasicInfo *dev, int devNum) {
    char devId[UDID_BUF_LEN];
    int ret, count = 0;
    for (int i = 0; i < devNum; i++) {
        ret = GetNodeKeyInfo(PACKAGE_NAME, dev[i].networkId, NODE_KEY_UDID, (uint8_t *)devId, UDID_BUF_LEN);
        if (ret == 0) {
            deviceDetail[i] = deviceIdAndInfo[devId];
            deviceDetail[i].deviceName = dev[i].deviceName;
            deviceDetail[i].networkId = dev[i].networkId;
            OH_LOG_INFO(LOG_APP, "[INFO] Get device info: {deviceName: %{public}s, deviceId: %{public}s}",
                        deviceDetail[i].deviceName.c_str(), dev[i].networkId);
            count++;
        }
    }
    return count;
}

/**
 * @brief 释放发现的组网内的设备
 * 
 * @param dev 要释放的设备的指针
 */
void FreeNodeInfoInterface(NodeBasicInfo *dev) {
    FreeNodeInfo(dev);
}

/**
 * @brief 通过SendBytes方式发送数据的封装接口
 * 
 * @param sessionId 和目标设备建立的session的id
 * @param data 发送的数据
 * @param len 发送数据的长度
 * @return int 成功返回0，否则返回1
 */
int SendBytesInterface(int sessionId, const void *data, unsigned int len) {
    int ret;
    ret = SendBytes(sessionId, data, len);
    if (ret) {
        OH_LOG_INFO(LOG_APP, "[INFO] SendBytes fail:%{public}d", ret);
    }
    return ret;
}

/***
 * 获取当前时间
 * @return 
 */
string getCurrentTime() {
    time_t rawTime;
    time(&rawTime);
    struct tm *timeInfo = localtime(&rawTime);
    char buffer[256];
    strftime(buffer, 256, "%Y-%m-%d-%H-%M-%S", timeInfo);
    return buffer;
}
