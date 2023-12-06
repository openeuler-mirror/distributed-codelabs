#include "device_manager.h"
#include "hilog/log.h"
#include "device_auth_client.h"

#include <atomic>

using namespace OHOS::DistributedHardware;

static inline OHOS::HiviewDFX::HiLogLabel LogLabel() {
    return {LOG_APP, 0xfffd, "dm_demo"};
}

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "[DM_DEMO]"

#define MY_LOGD(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Debug(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define MY_LOGI(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Info(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define MY_LOGW(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Warn(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#define MY_LOGE(fmt, ...) \
        OHOS::HiviewDFX::HiLog::Error(LogLabel(), LOG_TAG "::%{public}s: " fmt, __FUNCTION__, ##__VA_ARGS__)


class DmInitCallbackImpl : public DmInitCallback {
public:
    explicit DmInitCallback() {}
    virtual DmInitCallback() {}
    void OnRemoteDied() override {}
};

class DeviceStateCallbackImpl : public DeviceStateCallback {
public:
    void OnDeviceOnline(const DmDeviceInfo &deviceInfo) override {
        ++onlineNum_;
        MY_LOGI("device online: %s. current online num: %d", deviceInfo.deviceId, onlineNum_.load());
    }

    void OnDeviceOffline(const DmDeviceInfo &deviceInfo) override {
        --onlineNum_;
        MY_LOGI("device offline: %s. current online num: %d", deviceInfo.deviceId, onlineNum_.load());
    }

    void OnDeviceChanged(const DmDeviceInfo &deviceInfo) override {
        MY_LOGI("device changed: %s", deviceInfo.deviceId);
    }

    void OnDeviceReady(const DmDeviceInfo &deviceInfo) override {
        MY_LOGI("device ready: %s", deviceInfo.deviceId);
    }
private:
    std::atomic<int32_t> onlineNum_ = 0;
};

class DeviceDiscoveryCallbackImpl : public DiscoveryCallback {
public:
    DeviceDiscoveryCallbackImpl() : DiscoveryCallback() {}
    virtual ~DeviceDiscoveryCallbackImpl() {}
    void OnDiscoverySuccess(uint16_t subscribeId) override {
        MY_LOGI("Success to discovery, subcribeId is %d.", subscribeId);
    }
    void OnDiscoveryFailed(uint16_t subscribeId, int32_t failedReason) override {
        MY_LOGI("Failed to discovery, subcribeId is %d, failedReason is %d.", subscribeId, failedReason);
    }
    void OnDeviceFound(uint16_t subscribeId, const DmDeviceInfo &deviceInfo) override {
        MY_LOGI("device found: %s", deviceInfo.deviceId);
    }
};

void DeviceAuthClient::Init() {
    MY_LOGI("Begin to device auth...");

    std::string bundleName = "dm_demo";
    auto initCb = std::make_shared<DmInitCallbackImpl>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(bundleName, initCb);
    if (ret != 0) {
        MY_LOGE("Init device manager failed, ret is %d.", ret);
        return;
    }
    MY_LOGI("Init device manager success.");

    auto stateCb = std::make_shared<DeviceStateCallbackImpl>();
    ret = DeviceManager::GetInstance().RegisterDevStateCallback(bundleName, "", stateCb);
    if (ret != 0) {
        MY_LOGE("Register device state callback failed, ret is %d.", ret);
        return;
    }
    MY_LOGI("Register device state callback success.");


    DmSubscribeInfo subscribeInfo = {
        .subscribeId = 123,
        .capability = "osdCapability",
        .freq = DM_MID,
        .isSameAccount = false,
        .isWakeRemote = false,
        .medium = DM_COAP
    };

    
    ret = DeviceManager::GetInstance().StartDeviceDiscovery(bundleName, );

}