#ifndef __DEVICE_AUTH_CLIENT__H__
#define __DEVICE_AUTH_CLIENT__H__

#include "device_manager.h"

class DeviceAuthClient {
public:
    DeviceAuthClient() = default;
    ~DeviceAuthClient() = default;

    void Init();
    void DeInit();
};

#endif  //!__DEVICE_AUTH_CLIENT__H__