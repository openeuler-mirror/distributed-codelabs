#define LOG_TAG "[SOFTBUS_DEMO]"

#include "softbus_adapter.h"
#include "softbus_error_code.h"
#include "softbus_common.h"
#include "hilog/log.h"
#include "session.h"

#include <sstream>
#include <functional>
#include <fstream>
#include <thread>
#include <chrono>
#include <stdlib.h>
#include <time.h>

#ifdef  ACCESS_TOKEN
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "accesstoken_kit.h"
#endif  //ACCESS_TOKEN

// hilog日志打印使用
static inline OHOS::HiviewDFX::HiLogLabel LogLabel() {
    return {LOG_APP, 0xfffe, "SOFTBUS_DEMO"};
}

#define MY_LOGD(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Debug(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define MY_LOGI(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Info(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define MY_LOGW(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Warn(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define MY_LOGE(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Error(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define LOG_RESULT(ret) \
    do { \
        if (ret == SoftBusErrNo::SOFTBUS_OK) { \
            MY_LOGI("Succeed to %s", __func__); \
        } else { \
            MY_LOGE("Failed to %s, ret = %d", __func__, ret); \
        } \
    } while (0)

#define SESSION_ID_FILE "/etc/SI"
#define SESSION_ID_LEN 65

const std::string PACKAGE_NAME = "softbus_sample";
const std::string LOCAL_SESSION_NAME = "session_test";
const std::string TARGET_SESSION_NAME = "session_test";
const std::string DEFAULT_CAPABILITY = "osdCapability";
const std::string DEFAULT_SESSION_GROUP = "group_test";
const int DEFAULT_PUBLISH_ID = 123;

std::unordered_map<int, std::string> SoftbusAdapter::sessionNetMap = {};
std::unordered_map<std::string, int> SoftbusAdapter::netSessionMap = {};

static void DeviceFoundCallback(const DeviceInfo *device)
{
    std::ostringstream infoStr;

    MY_LOGI("Find a new device:");
    infoStr << "\n\tdevId=" << device->devId << "\n";
    infoStr << "\taccountHash=" << device->accountHash << "\n";
    infoStr << "\tdevType=" << device->devType << "\n";
    infoStr << "\tdevName=" << device->devName << "\n";
    infoStr << "\tisOnline=" << (device->isOnline ? "true" : "false") << "\n";
    infoStr << "\taddrNum=" << device->addrNum << "\n";

    for (unsigned int i = 0; i < device->addrNum; i++) {
        infoStr << "\t\taddr" << i + 1 << ":type=" << device->addr[i].type << ",";
        switch (device->addr[i].type) {
            case CONNECTION_ADDR_WLAN:
            case CONNECTION_ADDR_ETH:
                infoStr << "ip=" << device->addr[i].info.ip.ip << ",port=" << device->addr[i].info.ip.port << ",";
                break;
            default:
                break;
        }
        infoStr << "peerUid=" << device->addr[i].peerUid << "\n";
    }

    infoStr << "\tcapabilityBitmapNum=" << device->capabilityBitmapNum << "\n";
    for (unsigned int i = 0; i < device->capabilityBitmapNum; i++) {
        infoStr << "\t\tcapabilityBitmap[" << i + 1 << "]=0x" << std::hex << device->capabilityBitmap[i] << "\n";
    }

    infoStr << "\tcustData=" << device->custData << "\n";

    MY_LOGI("%s", infoStr.str().c_str());
}

SoftbusAdapter::SoftbusAdapter()
{
    _capability = DEFAULT_CAPABILITY;
    std::ifstream fin;
    fin.open(SESSION_ID_FILE, std::ios::in);
    if (!fin.is_open()) {
        MY_LOGE("file %s open failed", SESSION_ID_FILE);
        _packName = PACKAGE_NAME;
        _sessionName = LOCAL_SESSION_NAME;
        _sessionGroup = DEFAULT_SESSION_GROUP;
        _publishId = DEFAULT_PUBLISH_ID;
        fin.close();
        return;
    }
    char si[SESSION_ID_LEN] = {0};
    fin >> si;
    MY_LOGE("file %s read %s", SESSION_ID_FILE, si);
    _packName = si;
    _sessionName = si;
    _sessionGroup = si;
    _publishId = GetRandPublishID();
    fin.close();
}

int SoftbusAdapter::GetRandPublishID() {
    int publishId;
    srand((unsigned)time(nullptr));
    publishId = SESSION_ID_LEN * 2 + rand() % SESSION_ID_LEN;
    MY_LOGE("_publishId %d", publishId);
    return publishId;
}

SoftbusAdapter::~SoftbusAdapter()
{
}

#ifdef ACCESS_TOKEN
void SoftbusAdapter::AddPermission()
{
    uint64_t tokenId;
    const char *perms[2];
    perms[0] = OHOS_PERMISSION_DISTRIBUTED_DATASYNC;
    perms[1] = OHOS_PERMISSION_DISTRIBUTED_SOFTBUS_CENTER;
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "softbus_client",
        .aplStr = "normal",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}
#endif //ACCESS_TOKEN

void SoftbusAdapter::Init()
{
#ifdef  ACCESS_TOKEN
    AddPermission();
#endif  //ACCESS_TOKEN

    // PublishServiceAdapt();
    // StartDiscoveryAdapt();
    PublishLNNAdapt();
    RefreshLNNAdapt();
    CreateSessionServerAdapt();
    SetFileListener();
}

void SoftbusAdapter::Exit()
{
    RemoveSessionServerAdapt();
    StopRefreshLNNAdapt();
    StopPublishLNNAdapt();
    // StopDiscoveryAdapt();
    // UnPublishServiceAdapt();
}

int SoftbusAdapter::PublishLNNAdapt(IPublishCb *cb)
{
    PublishInfo info = {
        .publishId = _publishId,
        .mode = DISCOVER_MODE_PASSIVE,
        .medium = COAP,
        .freq = LOW,
        .capability = _capability.c_str(),
        .capabilityData = NULL,
        .dataLen = 0,
    };
    if (cb == nullptr) {
        cb = new IPublishCb {
            .OnPublishResult = [](int publishId, PublishResult reason) {
                MY_LOGI("Publish %d LNN ret: %d.", publishId, (int)reason);
            }
        };
    }
    int ret = PublishLNN(_packName.c_str(), &info, cb);
    LOG_RESULT(ret);
    delete cb;
    cb = nullptr;
    return ret;
}

int SoftbusAdapter::StopPublishLNNAdapt()
{
    int ret = StopPublishLNN(_packName.c_str(), _publishId);
    LOG_RESULT(ret);
    return ret;
}

// 此接口官方文档已不推荐，推荐使用PublishLNN
int SoftbusAdapter::PublishServiceAdapt(IPublishCallback *cb)
{
    PublishInfo info = {
        .publishId = _publishId,
        .mode = DISCOVER_MODE_PASSIVE,
        .medium = COAP,
        .freq = LOW,
        .capability = _capability.c_str(),
        .capabilityData = NULL,
        .dataLen = 0,
    };
    if (cb == nullptr) {
        cb = new IPublishCallback {
            .OnPublishSuccess = [](int publishId) {
                MY_LOGI("Success to publish %d.", publishId);
            },
            .OnPublishFail = [](int publishId, PublishFailReason reason) {
                MY_LOGE("Failed to publish %d, reason is: %d.", publishId, (int)reason);
            }
        };
    }

    int ret = PublishService(_packName.c_str(), &info, reinterpret_cast<const IPublishCallback *>(cb));
    LOG_RESULT(ret);
    delete cb;
    cb = nullptr;
    return ret;
}

// 此接口官方文档已不推荐，推荐使用StopPublishLNN
int SoftbusAdapter::UnPublishServiceAdapt()
{
    int ret = UnPublishService(_packName.c_str(), _publishId);
    LOG_RESULT(ret);
    return ret;
}

int SoftbusAdapter::RefreshLNNAdapt(IRefreshCallback *cb)
{
    SubscribeInfo info = {
        .subscribeId = _publishId,
        .mode = DISCOVER_MODE_ACTIVE,
        .medium = COAP,
        .freq = LOW,
        .isSameAccount = false,
        .isWakeRemote = false,
        .capability = _capability.c_str(),
        .capabilityData = NULL,
        .dataLen = 0,
    };

    if (cb == nullptr) {
        cb = new IRefreshCallback {
            .OnDeviceFound = DeviceFoundCallback,
            .OnDiscoverResult = [](int32_t refreshId, RefreshResult reason) {
                MY_LOGI("Refresh %d LNN ret: %d.", refreshId, (int)reason);
            }
        };
    }

    int ret = RefreshLNN(_packName.c_str(), &info, cb);
    LOG_RESULT(ret);
    delete cb;
    cb = nullptr;
    return ret;
}

int SoftbusAdapter::StopRefreshLNNAdapt()
{
    // TODO: 确认一下形参的refreshId和publishId是否一个
    int ret = StopRefreshLNN(_packName.c_str(), _publishId);
    LOG_RESULT(ret);
    return ret;
}

// 此接口官方文档已不推荐，推荐使用RefreshLNN
int SoftbusAdapter::StartDiscoveryAdapt(IDiscoveryCallback *cb)
{
    SubscribeInfo info = {
        .subscribeId = _publishId,
        .mode = DISCOVER_MODE_ACTIVE,
        .medium = COAP,
        .freq = LOW,
        .isSameAccount = false,
        .isWakeRemote = false,
        .capability = _capability.c_str(),
        .capabilityData = NULL,
        .dataLen = 0,
    };

    if (cb == nullptr) {
        cb = new IDiscoveryCallback {
            .OnDeviceFound = DeviceFoundCallback,
            .OnDiscoverFailed = [](int subscribeId, DiscoveryFailReason reason) {
                MY_LOGE("Failed to discover subscribeId=%d, reason=%d.", subscribeId, (int)reason);
            },
            .OnDiscoverySuccess = [](int subscribeId) {
                MY_LOGI("Success to discover subscribeId=%d.", subscribeId);
            },
        };
    }

    int ret = StartDiscovery(_packName.c_str(), &info, cb);
    LOG_RESULT(ret);
    delete cb;
    cb = nullptr;
    return ret;
}

// 此接口官方文档已不推荐，推荐使用StopRefreshLNN
int SoftbusAdapter::StopDiscoveryAdapt()
{
    int ret = StopDiscovery(_packName.c_str(), _publishId);
    LOG_RESULT(ret);
    return ret;
}

int SoftbusAdapter::CreateSessionServerAdapt(ISessionListener *cb)
{
    if (cb == nullptr) {
        // TODO: map的写需要加锁
        cb = new ISessionListener {
            .OnSessionOpened = [](int sessionId, int result) -> int {
                if (result != SOFTBUS_OK) {
                    MY_LOGE("Session %d opened failed, ret is %d.", sessionId, result);
                    return result;
                }
                MY_LOGI("Sessionid [%d] opened, ret is %d.", sessionId, result);
                char *networkId = new char[DISC_MAX_DEVICE_ID_LEN];
                if (GetPeerDeviceId(sessionId, networkId, DISC_MAX_DEVICE_ID_LEN) == SOFTBUS_OK) {
                    MY_LOGI("Success get peer(%s) device id.", networkId);
                    sessionNetMap[sessionId] = networkId;
                    netSessionMap[networkId] = sessionId;
                }
                delete[] networkId;
                return result;
            },
            .OnSessionClosed = [](int sessionId) {
                MY_LOGI("Session %d closed.", sessionId);
                std::string networkId =  sessionNetMap[sessionId];
                sessionNetMap.erase(sessionId);
                netSessionMap.erase(networkId);
            },
            .OnBytesReceived = [](int sessionId, const void *data, unsigned int dataLen) {
                MY_LOGI("Session %d received %u bytes from %s:", sessionId, dataLen, sessionNetMap[sessionId].c_str());
                // std::string temp((const char *)data, dataLen);
                // MY_LOGI("%s", temp.c_str());
                MY_LOGI("%.*s", dataLen, (const char*)data);
            },
            .OnMessageReceived = [](int sessionId, const void *data, unsigned int dataLen) {
                MY_LOGI("Session %d received %u bytes:", sessionId, dataLen);
                // std::string temp((const char *)data, dataLen);
                MY_LOGI("%.*s", dataLen, (const char*)data);
            },
            .OnStreamReceived = [](int sessionId, const StreamData *data, const StreamData *ext, const StreamFrameInfo *param) {
                MY_LOGI("Session %d received stream, dataLen %d:", sessionId, data->bufLen);
                std::string temp((const char *)data->buf, data->bufLen);
                MY_LOGI("%s", temp.c_str());
                // CloseSessionAdapt();
                // CloseSession(sessionId);
                // MY_LOGI("Closed session %d.", sessionId);
            }
        };
    }

    int ret = CreateSessionServer(_packName.c_str(), _sessionName.c_str(), cb);
    LOG_RESULT(ret);
    delete cb;
    cb = nullptr;
    return ret;
}

int SoftbusAdapter::SetFileListener(IFileReceiveListener *recvCb, IFileSendListener *sendCb)
{
    if (recvCb == nullptr) {
        recvCb = new IFileReceiveListener {
            .OnReceiveFileStarted = [](int sessionId, const char *files, int fileCnt) -> int {
                MY_LOGI("[Start] Session %d receives %d files:", sessionId, fileCnt);
                // for (int i = 0; i < fileCnt; ++i) {
                //     MY_LOGI("%d: %s", i, files[i]);
                // }
                return 0;
            },
            .OnReceiveFileProcess = [](int sessionId, const char *firstFile, uint64_t bytesUpload, uint64_t bytesTotal) -> int {
                MY_LOGI("[Process] Session %d receiving %s: %llu/%llu", sessionId, firstFile, bytesUpload, bytesTotal);
                return 0;
            },
            .OnReceiveFileFinished = [](int sessionId, const char *files, int fileCnt) {
                MY_LOGI("[Finished] Session %d received %d files", sessionId, fileCnt);
                // for (int i = 0; i < fileCnt; ++i) {
                //     MY_LOGI("%d: %s", i, files[i]);
                // }
            },
            .OnFileTransError = [](int sessionId) {
                MY_LOGE("[Error] Session %d receive file failed.", sessionId);
            }
        };
    }

    int ret = SetFileReceiveListener(_packName.c_str(), _sessionName.c_str(), recvCb, "/tmp");
    LOG_RESULT(ret);
    delete recvCb;
    recvCb = nullptr;

    if (sendCb == nullptr) {
        sendCb = new IFileSendListener {
            .OnSendFileProcess = [](int sessionId, uint64_t bytesUpload, uint64_t bytesTotal) -> int {
                MY_LOGI("[Process] Session %d sending: %llu/%llu", sessionId,  bytesUpload, bytesTotal);
                return 0;
            },
            .OnSendFileFinished = [](int sessionId, const char *firstFile) -> int {
                MY_LOGI("[Finished] Session %d send %s success.", sessionId, firstFile);
                return 0;
            },
            .OnFileTransError = [](int sessionId) {
                MY_LOGE("[Error] Session %d send file failed.", sessionId);
            }
        };
    }

    ret |= SetFileSendListener(_packName.c_str(), _sessionName.c_str(), sendCb);
    LOG_RESULT(ret);
    delete sendCb;
    sendCb = nullptr;

    return ret;
}

int SoftbusAdapter::RemoveSessionServerAdapt()
{
    int ret = RemoveSessionServer(_packName.c_str(), _sessionName.c_str());
    LOG_RESULT(ret);
    return ret;
}

bool SoftbusAdapter::IsDeviceConnected(const std::string &deviceId)
{
    return (netSessionMap.find(deviceId) != netSessionMap.end());
}

int SoftbusAdapter::OpenSessionAdapt(const char *peerNetworkId, const char *targetSessionName,
                                     const int dataType) {
    SessionAttribute attr = {
        .dataType = dataType,
        .linkTypeNum = 1,
        .linkType = {LINK_TYPE_WIFI_WLAN_2G},
        .attr = {RAW_STREAM},
    };

    if (IsDeviceConnected(std::string(peerNetworkId))) {
        MY_LOGI("device %s is already connected.", peerNetworkId);
        return netSessionMap[peerNetworkId];
    }

    int sessionId = OpenSession(_sessionName.c_str(), targetSessionName, peerNetworkId,
                          _sessionGroup.c_str(), &attr);
    if (sessionId <= 0) {
        MY_LOGE("Failed to open session with peer(%s).", peerNetworkId);
        return 0;
    }
    // OpenSession的返回值成功不代表真的open session成功了
    // sessionNetMap[sessionId] = peerNetworkId;
    // netSessionMap[peerNetworkId] = sessionId;
    MY_LOGI("Success to invoke opensession with peer(%s).", peerNetworkId);
    return sessionId;
}

void SoftbusAdapter::CloseSessionAdapt(const int sessionId)
{
    CloseSession(sessionId);
    if (sessionNetMap.find(sessionId) == sessionNetMap.end()) {
        return;
    }
    std::string netId = sessionNetMap[sessionId];
    sessionNetMap.erase(sessionId);
    netSessionMap.erase(netId);
}

void  SoftbusAdapter::CloseSessionWithNetId(const std::string &peerNetworkId)
{
    if (netSessionMap.find(peerNetworkId) == netSessionMap.end()) {
        MY_LOGE("No opened session with device(%s).", peerNetworkId.c_str());
        return;
    }
    CloseSessionAdapt(netSessionMap[peerNetworkId]);
}

// 此函数会开辟内存存储devices的信息，需要调用者释放devices的内存空间
int SoftbusAdapter::GetAllDeviceInfo(NodeBasicInfo **devices)
{
    int ret, num;
    ret = GetAllNodeDeviceInfo(_packName.c_str(), devices, &num);
    if (ret) {
        MY_LOGE("Failed to get all node device info, ret is %d.", ret);
        return -1;
    }

    MY_LOGI("Success get %d node info.", num);
    return num;
}

void SoftbusAdapter::PrintAllDeviceInfo()
{
    NodeBasicInfo *devices = nullptr;
    int devNum = GetAllDeviceInfo(&devices);
    if (devNum <= 0) {
        MY_LOGI("No online device found.");
        return;
    }

    for (int idx = 0; idx < devNum; ++idx) {
        std::ostringstream output;
        output << idx << ": "
               << "name(" << devices[idx].deviceName << "), netId(" << devices[idx].networkId
               << "), type(" << devices[idx].deviceTypeId << "), ip("
               << GetIpByNetId(devices[idx].networkId) << ")." << std::endl;
        MY_LOGI("%s", output.str().c_str());
    }
    FreeNodeInfo(devices);
}

void SoftbusAdapter::PrintConnectedDevicesInfo()
{
    for (auto it = sessionNetMap.begin(); it != sessionNetMap.end(); ++it) {
        std::ostringstream output;
        output << "sessionId:" << it->first << ", networkId: " << it->second << std::endl;
        MY_LOGI("%s", output.str().c_str());
    }
}

int SoftbusAdapter::SendBytesAdapt(const int sessionId, const std::string &data)
{
    int ret = SendBytes(sessionId, data.c_str(), data.size());
    if (ret) {
        MY_LOGE("Failed to send data to sessionId(%d).", sessionId);
    }
    return ret;
}

int SoftbusAdapter::SendStreamAdapt(const int sessionId, const std::string &data)
{
    StreamData tempData = {
        .buf = (char *)data.c_str(),
        .bufLen = (int)data.size(),
    };
    StreamData extData = {0};
    StreamFrameInfo frameInfo = {0};
    int ret = SendStream(sessionId, &tempData, &extData, &frameInfo);
    // if (ret) {
    //     MY_LOGE("Failed to send stream data to session(%d).", sessionId);
    // }
    LOG_RESULT(ret);
    return ret;
}

int SoftbusAdapter::SendStreamToAllDevice(const std::string &data)
{
    NodeBasicInfo *devices = nullptr;
    int devNum = GetAllDeviceInfo(&devices);
    if (devNum <= 0) {
        MY_LOGI("No online device found.");
        return -1;
    }

    for (int idx = 0; idx < devNum; ++idx) {
        char *netId = devices[idx].networkId;
        int sessionId = 0;
        if (!IsDeviceConnected(std::string(netId))) {
            MY_LOGI("Device(%s) is disconnected, try to connect it.", netId);
            sessionId = OpenSessionAdapt(netId, TARGET_SESSION_NAME.c_str(), TYPE_BYTES);

            std::chrono::milliseconds sleepDuration(200);
            std::this_thread::sleep_for(sleepDuration);
        } else {
            sessionId = netSessionMap[netId];
        }
        if ((sessionId == 0) || (IsDeviceConnected(std::string(netId)) == false)) {
            MY_LOGE("Device(%s) is disconnected, try other device.", netId);
            continue;
        }
        SendStreamAdapt(sessionId, data);
    }
    FreeNodeInfo(devices);
    return 0;
}

int SoftbusAdapter::OpenSessionWithAllDevice()
{
    NodeBasicInfo *devices = nullptr;
    int devNum = GetAllDeviceInfo(&devices);
    if (devNum <= 0) {
        MY_LOGI("No online device found.");
        return -1;
    }

    for (int idx = 0; idx < devNum; ++idx) {
        char *netId = devices[idx].networkId;
        OpenSessionAdapt(netId, TARGET_SESSION_NAME.c_str(), TYPE_BYTES);

        // 每次openSesion之后休息200ms
        // TODO: 后续去除，需要测试并发同时建链
        std::chrono::milliseconds sleepDuration(200);
        std::this_thread::sleep_for(sleepDuration);
    }
    return 0;
}

int SoftbusAdapter::SendFileAdapt(const int sessionId, const char *fileList[], const uint32_t fileCnt)
{
    int ret = SendFile(sessionId, fileList, fileList, fileCnt);
    LOG_RESULT(ret);
    return ret;
}

int SoftbusAdapter::SendBytesToAllDevice(const std::string &data)
{
    NodeBasicInfo *devices = nullptr;
    int devNum = GetAllDeviceInfo(&devices);
    if (devNum <= 0) {
        MY_LOGI("No online device found.");
        return -1;
    }

    for (int idx = 0; idx < devNum; ++idx) {
        char *netId = devices[idx].networkId;
        int sessionId = 0;
        if (!IsDeviceConnected(std::string(netId))) {
            MY_LOGI("Device(%s) is disconnected, try to connect it.", netId);
            sessionId = OpenSessionAdapt(netId, TARGET_SESSION_NAME.c_str(), TYPE_BYTES);

            std::chrono::milliseconds sleepDuration(200);
            std::this_thread::sleep_for(sleepDuration);
        } else {
            sessionId = netSessionMap[netId];
        }
        if ((sessionId == 0) || (IsDeviceConnected(std::string(netId)) == false)) {
            MY_LOGE("Device(%s) is disconnected, try other device.", netId);
            continue;
        }
        SendBytesAdapt(sessionId, data);
    }
    FreeNodeInfo(devices);
    return 0;
}

std::string SoftbusAdapter::GetNetIdBySessionId(const int sessionId)
{
    if (sessionNetMap.find(sessionId) != sessionNetMap.end()) {
        return sessionNetMap[sessionId];
    }
    return "";
}

std::string SoftbusAdapter::GetIpByNetId(const std::string &networkId)
{
    char ip[IP_STR_MAX_LEN];
    int ret = GetNodeKeyInfo(_packName.c_str(), networkId.c_str(), NODE_KEY_IP_ADDRESS, (uint8_t *)ip, IP_STR_MAX_LEN);
    if (ret != 0) {
        MY_LOGE("Failed to get %s ip.", networkId.c_str());
        return "";
    }
    return std::string(ip);
}

std::string SoftbusAdapter::GetLocalNetId()
{
    NodeBasicInfo localNodeInfo;
    GetLocalNodeDeviceInfo(_packName.c_str(), &localNodeInfo);
    return std::string(localNodeInfo.networkId);
}
