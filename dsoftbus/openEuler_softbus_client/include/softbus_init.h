#include <stdio.h>
#include <unistd.h>
#include "securec.h"
#include "discovery_service.h"
#include "softbus_bus_center.h"
#include "session.h"

#define UN_OPENED_SESSIONID -1

struct SoftClientInitParam
{
    const char *package_name;
    const char *local_session_name;
    const char *target_session_name;
    const char *default_capability;
    const char *default_session_group;
    int default_publish_id;
    int opened_sessionId;
};

void logger(const char *__restrict __fmt, ...);

void DefaultPublishSuccess(int publishId);

void DefaultPublishFailed(int publishId, PublishFailReason reason);

void UnPublishServiceInterface(SoftClientInitParam *param);

void DeviceFound(const DeviceInfo *device);

void DiscoverySuccess(int subscribeId);

void DiscoveryFailed(int subscribeId, DiscoveryFailReason reason);

void StopDiscoveryInterface(SoftClientInitParam *param);

void SessionClosed(int sessionId);

void ByteRecived(int sessionId, const void *data, unsigned int dataLen);

void MessageReceived(int sessionId, const void *data, unsigned int dataLen);

void RemoveSessionServerInterface(SoftClientInitParam *param);

int OpenSessionInterface(SoftClientInitParam *param, const char *peerNetworkId);

void CloseSessionInterface(int sessionId);

int GetAllNodeDeviceInfoInterface(SoftClientInitParam *param, NodeBasicInfo **dev);

void FreeNodeInfoInterface(NodeBasicInfo *dev);

void commnunicate(SoftClientInitParam *param);

int testSoftbus(SoftClientInitParam *param, int (*SessionOpened)(int sessionId, int result));

int PublishServiceInterface(SoftClientInitParam *param,
                            void (*PublishSuccess)(int publishId) = DefaultPublishSuccess,
                            void (*PublishFailed)(int publishId, PublishFailReason reason) = DefaultPublishFailed);

int DiscoveryInterface(SoftClientInitParam *param,
                       void (*OnDeviceFound)(const DeviceInfo *device) = DeviceFound,
                       void (*OnDiscoverFailed)(int subscribeId, DiscoveryFailReason failReason) = DiscoveryFailed,
                       void (*OnDiscoverySuccess)(int subscribeId) = DiscoverySuccess);

int CreateSessionServerInterface(SoftClientInitParam *param,
                                 int (*SessionOpened)(int sessionId, int result),
                                 void (*SessionClosed)(int sessionId) = SessionClosed,
                                 void (*ByteRecived)(int sessionId, const void *data, unsigned int dataLen) = ByteRecived,
                                 void (*MessageReceived)(int sessionId, const void *data, unsigned int dataLen) = MessageReceived);
