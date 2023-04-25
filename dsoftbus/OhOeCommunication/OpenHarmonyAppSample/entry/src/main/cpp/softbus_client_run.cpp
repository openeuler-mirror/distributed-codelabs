#include <cstring>
#include <string>
#include <sstream>
#include <map>
#include <dirent.h>
#include <sys/stat.h>
#include <js_native_api.h>
#include <js_native_api_types.h>
#include <unistd.h>
#include "discovery_service.h"
#include "softbus_client_interface.h"
#include "napi/native_api.h"
#include "hilog/log.h"

using namespace std;

static napi_env envRef;
static napi_ref callbackRef;
NodeBasicInfo *dev;
static int openedSessionId;

static void sessionDisconnect(int stepNum);

static string getConnectedDeviceInfo(DeviceDetail *deviceDetails, int arrayLength);

/**
 * 建立连接前的初始化功能，包括创建sessionServer，发布软总线能力，发现同局域网下的设备等。
 * @return 初始化的结果。
 */
static string sessionServerInit(callBack *func) {
    int ret;

    // 创建sessionServer
    ret = CreateSessionServerInterface(func);
    if (ret) {
        OH_LOG_ERROR(LOG_APP, "[ERROR] <CreateSessionServerInterface> failed");
        return "<CreateSessionServerInterface> failed";
    }
    // 发布服务
    ret = PublishServiceInterface();
    if (ret) {
        OH_LOG_ERROR(LOG_APP, "[ERROR] <PublishServiceInterface> failed");
        sessionDisconnect(4);
        return "<PublishServiceInterface> failed";
    }
    // 启动探测
    ret = DiscoveryInterface();
    if (ret) {
        OH_LOG_ERROR(LOG_APP, "[ERROR] <DiscoveryInterface> failed");
        sessionDisconnect(3);
        return "<DiscoveryInterface> failed";
    }

    OH_LOG_INFO(LOG_APP, "[INFO] Session server init success!!!");
    return "[INFO] Session server init success";
}

/**
 * 发现组网内设备
 * @return 建立连接的设备信息
 */
static string sessionConnect() {

    // 重新探测设备
    StopDiscoveryInterface();
    DiscoveryInterface();

    openedSessionId = -1;
    dev = nullptr;
    int devNum = GetAllNodeDeviceInfoInterface(&dev); //返回组网内的设备数
    if (devNum <= 0) {
        OH_LOG_ERROR(LOG_APP, "[ERROR] <GetAllNodeDeviceInfoInterface> failed");
        return "<GetAllNodeDeviceInfoInterface> failed";
    }

    DeviceDetail deviceDetails[devNum];
    int connectNum = GetDeviceDetailInterface(deviceDetails, dev, devNum);
    OH_LOG_INFO(LOG_APP, "[INFO] Connected device count: %{public}d", connectNum);

    return getConnectedDeviceInfo(deviceDetails, connectNum);
}

/***
 * 设备连接失败，或者手动关闭连接时调用，用于释放资源
 * @param stepNum 失败的步骤
 * @param sessionId 需要关闭的session id,默认为-1,表示使用不到该参数。
 */
static void sessionDisconnect(int stepNum) {
    switch (stepNum) {
    case 0:
        // 关闭session连接
        // closeAllSession();
        OH_LOG_INFO(LOG_APP, "[INFO] Session close end!!!");
    case 1:
        // 释放节点信息
        FreeNodeInfoInterface(dev);
        OH_LOG_INFO(LOG_APP, "[INFO] Node free end!!!");
    case 2:
        // 取消探测
        StopDiscoveryInterface();
        OH_LOG_INFO(LOG_APP, "[INFO] Discovery stop end!!!");
    case 3:
        // 关闭发布特定服务能力
        UnPublishServiceInterface();
        OH_LOG_INFO(LOG_APP, "[INFO] Service UnPublish end!!!");
    case 4:
        // 移除sever
        RemoveSessionServerInterface();
        OH_LOG_INFO(LOG_APP, "[INFO] SessionServer remove end!!!");
        break;
    default:
        // 不知执行到哪一步时，全部关闭。
        // closeAllSession();
        FreeNodeInfoInterface(dev);
        StopDiscoveryInterface();
        UnPublishServiceInterface();
        RemoveSessionServerInterface();
    }
}

/**
 * 通过分布式软总线发送消息
 * @param sessionId 对端设备的session id
 * @param data 发送的数据
 * @param dataLength 发送数据的长度
 * @return 发送是否成功，返回0为成功，否则为不成功
 */
static int sendData(const char *peerSessionName, const char *data, unsigned int dataLength) {
    if (strcmp(peerSessionName, "session_test") != 0 || openedSessionId <= 0 || !data || dataLength <= 0) {
        OH_LOG_ERROR(LOG_APP, "[ERROR] peerSessionName:  %{public}s or sessionId: %{public}d or data: %{public}s or dataLength: %{public}d is not available",
                     peerSessionName, openedSessionId, data, dataLength);
        return -1;
    }
    int ret = SendBytesInterface(openedSessionId, data, dataLength);
    if (ret) {
        OH_LOG_ERROR(LOG_APP, "[ERROR] Failed to send data to sessionId: %{public}d Return value %{public}d ", openedSessionId, ret);
        return ret;
    }
    OH_LOG_INFO(LOG_APP, "[INFO] Successfully send data to the device of : %{public}s", peerSessionName);
    return ret;
}

/**
 * @brief 组装需要回传展示的设备信息
 * 
 * @param deviceDetails 通过GetDeviceDetailInterface接口获取的设备信息
 * @param connectNum 设备数量
 * @return string 组装的设备信息
 */
static string getConnectedDeviceInfo(DeviceDetail *deviceDetails, int connectNum) {
    stringstream fmt;
    for (int i = 0; i < connectNum; i++) {
        fmt << "deviceName: " << deviceDetails[i].deviceName << "\n"
            << "ipAddress: " << deviceDetails[i].ipaddress << "\n"
            << "port: " << deviceDetails[i].port << ";\n";
    }
    return fmt.str();
}

/**
 * @brief 接收数据后App的回调函数
 * 
 * @param receiveData 其他设备传输的数据
 */
static void receiveCallback(string receiveData) {
    OH_LOG_INFO(LOG_APP, "[INFO] Ets callback is call");
    napi_value jsReceiveData, jsCallback = nullptr, jsResult = nullptr;
    napi_create_string_utf8(envRef, receiveData.c_str(), receiveData.length(), &jsReceiveData);

    napi_get_reference_value(envRef, callbackRef, &jsCallback);
    napi_call_function(envRef, nullptr, jsCallback, 1, &jsReceiveData, &jsResult);
    OH_LOG_INFO(LOG_APP, "[INFO] Ets callback end");
}

/**
 * session服务初始化的napi封装接口
 */
static napi_value sessionServerInitC(napi_env env, napi_callback_info info) {
    envRef = env;
    size_t argc = 1;
    napi_value args[argc];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    napi_valuetype callbackType = napi_undefined;
    napi_typeof(env, args[0], &callbackType);
    napi_create_reference(env, args[0], 1, &callbackRef);

    napi_value initResult;
    string result = sessionServerInit(receiveCallback);
    napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &initResult);
    return initResult;
}

/**
 * session连接的napi封装接口
 */
static napi_value sessionConnectC(napi_env env, napi_callback_info info) {
    napi_value connectResult;
    string result = sessionConnect();
    napi_create_string_utf8(env, result.c_str(), NAPI_AUTO_LENGTH, &connectResult);
    return connectResult;
}

/**
 * 发送数据的napi封装接口
 */
static napi_value sendDataC(napi_env env, napi_callback_info info) {
    size_t bufferSize = 128;
    size_t copied;
    // 参数数量
    size_t argc = 3;
    // 声明参数数组
    napi_value args[3] = {nullptr};
    // 获取传入的参数并依次放入参数数组中
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // 对第一个js参数类型进行判定
    napi_valuetype valueType0;
    napi_typeof(env, args[0], &valueType0);
    // 对第二个js参数类型进行判定
    napi_valuetype valueType1;
    napi_typeof(env, args[1], &valueType1);
    // 对第三个js参数类型进行判定
    napi_valuetype valueType2;
    napi_typeof(env, args[2], &valueType2);
    // 将第一个传入参数转化为string类型
    char value0Buffer[128];
    napi_get_value_string_utf8(env, args[0], value0Buffer, bufferSize, &copied);
    // 将第二个传入参数转化为string类型
    char value1Buffer[128];
    napi_get_value_string_utf8(env, args[1], value1Buffer, bufferSize, &copied);
    // 将第三个传入参数转化为int类型
    int value2;
    napi_get_value_int32(env, args[2], &value2);
    // 调用发送消息的函数
    napi_value sendResult;
    int result = sendData(value0Buffer, value1Buffer, value2);
    napi_create_int32(env, result, &sendResult);

    return sendResult;
}

/**
 * 关闭连接的napi封装接口
 */
static napi_value sessionDisconnectC(napi_env env, napi_callback_info info) {
    napi_value disconnectResult = nullptr;
    sessionDisconnect(0);
    return disconnectResult;
}

// napi函数注册
EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"sessionServerInit", nullptr, sessionServerInitC, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"sessionConnect", nullptr, sessionConnectC, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"sendData", nullptr, sendDataC, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"sessionDisconnect", nullptr, sessionDisconnectC, nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "libsoftbus_client_run",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterModule(void) {
    napi_module_register(&demoModule);
}
