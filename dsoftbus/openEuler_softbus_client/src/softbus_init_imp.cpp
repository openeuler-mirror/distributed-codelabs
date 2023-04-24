#include <iostream>
#include <memory>
#include "softbus_init.h"


void logger(const char *__restrict __fmt, ...) {
    printf(__fmt);
}

void DefaultPublishSuccess(int publishId)
{
    logger("<PublishSuccess>CB: publish %d done", publishId);
}

void DefaultPublishFailed(int publishId, PublishFailReason reason)
{
    logger("<PublishFailed>CB: publish %d failed, reason= %d \n", publishId, (int)reason);
}

void UnPublishServiceInterface(SoftClientInitParam *param)
{
    int ret = UnPublishService(param->package_name, param->default_publish_id);
    if (ret != 0) {
        logger("UnPublishService fail:%d \n ", ret);
    }
}

void DeviceFound(const DeviceInfo *device)
{
    unsigned int i;
    logger("<DeviceFound>CB: Device has found\n");
    logger("\tdevId=%s\n", device->devId);
    logger("\tdevName=%s\n", device->devName);
    logger("\tdevType=%d\n", device->devType);
    logger("\taddrNum=%d\n", device->addrNum);
    for (i = 0; i < device->addrNum; i++) {
        logger("\t\taddr%d:type=%d,", i + 1, device->addr[i].type);
        switch (device->addr[i].type) { 
        case CONNECTION_ADDR_WLAN:
        case CONNECTION_ADDR_ETH:
            logger("ip=%s,port=%d,", device->addr[i].info.ip.ip, device->addr[i].info.ip.port);
            break;
        default:
            break;
        }
        logger("peerUid=%s\n", device->addr[i].peerUid);
    }
    logger("\tcapabilityBitmapNum=%d\n", device->capabilityBitmapNum);
    for (i = 0; i < device->addrNum; i++) {
        logger("\t\tcapabilityBitmap[%d]=0x%x\n", i + 1, device->capabilityBitmap[i]);
    }
    logger("\tcustData=%s\n", device->custData);
}

void DiscoverySuccess(int subscribeId)
{
    logger("<DiscoverySuccess>CB: discover subscribeId=%d\n", subscribeId);
}

void DiscoveryFailed(int subscribeId, DiscoveryFailReason reason)
{
    logger("<DiscoveryFailed>CB: discover subscribeId=%d failed, reason=%d\n", subscribeId, (int)reason);
}

void StopDiscoveryInterface(SoftClientInitParam *param)
{
    int ret = StopDiscovery(param->package_name, param->default_publish_id);
    if (ret) {
        logger("StopDiscovery fail:%d\n", ret);
    }
}

void SessionClosed(int sessionId)
{
    logger("<SessionClosed>CB: session %d closed\n", sessionId);
}

void ByteRecived(int sessionId, const void *data, unsigned int dataLen)
{
    const char *inputStr = (const char *)data;
    logger("<ByteRecived>CB: session %d received %u bytes data=%s\n", sessionId, dataLen, inputStr);
}

void MessageReceived(int sessionId, const void *data, unsigned int dataLen)
{
    logger("<MessageReceived>CB: session %d received %u bytes message=%s\n", sessionId, dataLen, (const char *)data);
}

void RemoveSessionServerInterface(SoftClientInitParam *param)
{
    int ret = RemoveSessionServer(param->package_name, param->local_session_name);
    if (ret) {
        logger("RemoveSessionServer fail:%d\n", ret);
    }
}

int OpenSessionInterface(SoftClientInitParam *param, const char *peerNetworkId)
{
    SessionAttribute attr = {
        .dataType = TYPE_BYTES,
        .linkTypeNum = 1,
        .attr = {RAW_STREAM},
    };
    attr.linkType[0] = LINK_TYPE_WIFI_WLAN_2G;
    return OpenSession(param->local_session_name, param->target_session_name, peerNetworkId, param->default_session_group, &attr);
}

void CloseSessionInterface(int sessionId)
{
    CloseSession(sessionId);
}

int GetAllNodeDeviceInfoInterface(SoftClientInitParam *param, NodeBasicInfo **dev)
{
    int ret, num;

    ret = GetAllNodeDeviceInfo(param->package_name, dev, &num);
    if (ret) {
        logger("GetAllNodeDeviceInfo fail:%d\n", ret);
        return -1;
    }

    logger("<GetAllNodeDeviceInfo>return %d Node\n", num);
    return num;
}

void FreeNodeInfoInterface(NodeBasicInfo *dev)
{
    FreeNodeInfo(dev);
}

void commnunicate(SoftClientInitParam *param)
{
    NodeBasicInfo *dev = NULL;
    char cData[] = "hello world test";
    int dev_num, sessionId, input, ret;
    int timeout = 5;

    dev_num = GetAllNodeDeviceInfoInterface(param, &dev);
    if (dev_num <= 0) {
        logger("no device online \n");
        return;
    }

    for (int i = 0; i < dev_num; i++) {
        char devId[UDID_BUF_LEN];
        logger("<num %d>deviceName=%s\n", i + 1, dev[i].deviceName);
        logger("\tnetworkId=%s\n", dev[i].networkId);
        if (GetNodeKeyInfo(param->package_name, dev[i].networkId, NODE_KEY_UDID, (uint8_t *)devId, UDID_BUF_LEN) == 0) {
            logger("\tdevId=%s\n", devId);
        }
        logger("\tType=%d\n", dev[i].deviceTypeId);
    }

    logger("\nInput Node num to commnunication:");
    scanf_s("%d", &input);
    if (input <= 0 || input > dev_num) {
        logger("error input num\n");
        goto err_input;
    }

    param->opened_sessionId = -1;
    sessionId = OpenSessionInterface(param, dev[input - 1].networkId);
    if (sessionId < 0) {
        logger("OpenSessionInterface fail, ret=%d\n", sessionId);
        goto err_OpenSessionInterface;
    }

    while (timeout) {
        if (param->opened_sessionId == sessionId) {
            logger("\nInput send bytes\n");
            scanf_s("%s", cData);
            ret = SendBytes(sessionId, cData, strlen(cData) + 1);
            if (ret) {
                logger("SendBytes fail:%d\n", ret);
            }
            break;
        }
        timeout--;
        sleep(1);
    }

    CloseSessionInterface(sessionId);
err_OpenSessionInterface:
err_input:
    FreeNodeInfoInterface(dev);
}

int PublishServiceInterface(SoftClientInitParam *param,
                            void (*PublishSuccess)(int publishId) /*= DefaultPublishSuccess*/,
                            void (*PublishFailed)(int publishId, PublishFailReason reason) /* = DefaultPublishFailed */)
{
    PublishInfo info = {
        .publishId = param->default_publish_id,
        .mode = DISCOVER_MODE_PASSIVE,
        .medium = COAP,
        .freq = LOW,
        .capability = param->default_capability,
        .capabilityData = NULL,
        .dataLen = 0,
    };
    IPublishCallback cb = {
        .OnPublishSuccess = PublishSuccess,
        .OnPublishFail = PublishFailed,
    };
    return PublishService(param->package_name, &info, &cb);
}

int DiscoveryInterface(SoftClientInitParam *param,
                       void (*OnDeviceFound)(const DeviceInfo *device) /* = DeviceFound */,
                       void (*OnDiscoverFailed)(int subscribeId, DiscoveryFailReason failReason) /* = DiscoveryFailed */,
                       void (*OnDiscoverySuccess)(int subscribeId) /* = DiscoverySuccess */)
{
    SubscribeInfo info = {
        .subscribeId = param->default_publish_id,
        .mode = DISCOVER_MODE_ACTIVE,
        .medium = COAP,
        .freq = LOW,
        .isSameAccount = false,
        .isWakeRemote = false,
        .capability = param->default_capability,
        .capabilityData = NULL,
        .dataLen = 0,
    };
    IDiscoveryCallback cb = {
        .OnDeviceFound = OnDeviceFound,
        .OnDiscoverFailed = OnDiscoverFailed,
        .OnDiscoverySuccess = OnDiscoverySuccess,
    };
    return StartDiscovery(param->package_name, &info, &cb);
}

int CreateSessionServerInterface(SoftClientInitParam *param,
                                 int (*SessionOpened)(int sessionId, int result),
                                 void (*SessionClosed)(int sessionId) /* = SessionClosed */,
                                 void (*ByteRecived)(int sessionId, const void *data, unsigned int dataLen) /* = ByteRecived */,
                                 void (*MessageReceived)(int sessionId, const void *data, unsigned int dataLen) /* = MessageReceived */)
{
    const ISessionListener sessionCB = {
        .OnSessionOpened = SessionOpened,
        .OnSessionClosed = SessionClosed,
        .OnBytesReceived = ByteRecived,
        .OnMessageReceived = MessageReceived,
    };

    return CreateSessionServer(param->package_name, param->local_session_name, &sessionCB);
}

int testSoftbus(SoftClientInitParam *param, int (*SessionOpened)(int sessionId, int result))
{
    bool loop = true;
    int ret;

    ret = CreateSessionServerInterface(param, SessionOpened);
    if (ret) {
        logger("CreateSessionServer fail, ret=%d\n", ret);
        return ret;
    }

    ret = PublishServiceInterface(param);
    if (ret) {
        logger("PublishService fail, ret=%d\n", ret);
        goto err_PublishServiceInterface;
    }

    ret = DiscoveryInterface(param);
    if (ret) {
        logger("DiscoveryInterface fail, ret=%d\n", ret);
        goto err_DiscoveryInterface;
    }

    while (loop) {
        logger("\nInput c to commnuication, Input s to stop:");
        char op = getchar();
        switch(op) {
        case 'c':
            commnunicate(param);
            continue;
        case 's':
            loop = false;
            break;
        case '\n':
            break;
        default:
            continue;
        }
    }

    StopDiscoveryInterface(param);
err_DiscoveryInterface:
    UnPublishServiceInterface(param);
err_PublishServiceInterface:
    RemoveSessionServerInterface(param);
    return 0;
}

