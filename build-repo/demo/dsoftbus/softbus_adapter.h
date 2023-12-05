#ifndef SOFTBUS_ADAPTER__H
#define SOFTBUS_ADAPTER__H

#include <string>
#include <unordered_map>

#include "discovery_service.h"
#include "softbus_bus_center.h"
#include "session.h"

class SoftbusAdapter
{
private:
    std::string _packName;
    std::string _sessionName;
    std::string _capability;
    std::string _sessionGroup;
    int _publishId;
    static std::unordered_map<int, std::string> sessionNetMap;
    static std::unordered_map<std::string, int> netSessionMap;
public:
    SoftbusAdapter();
    ~SoftbusAdapter();

    void Init();
    void Exit();

    int PublishLNNAdapt(IPublishCb *cb = nullptr);
    int StopPublishLNNAdapt();

    int PublishServiceAdapt(IPublishCallback *cb = nullptr);
    int UnPublishServiceAdapt();

    int RefreshLNNAdapt(IRefreshCallback *cb = nullptr);
    int StopRefreshLNNAdapt();

    int StartDiscoveryAdapt(IDiscoveryCallback *cb = nullptr);
    int StopDiscoveryAdapt();

    int CreateSessionServerAdapt(ISessionListener *cb = nullptr);
    int RemoveSessionServerAdapt();
    int SetFileListener(IFileReceiveListener *recvCb = nullptr, IFileSendListener *sendCb = nullptr);

    int OpenSessionAdapt(const char *peerNetworkId, const char* targetSessionName, const int dataType);
    int OpenSessionWithAllDevice();
    void CloseSessionAdapt(const int sessionId);
    void CloseSessionWithNetId(const std::string &peerNetworkId);

    int GetAllDeviceInfo(NodeBasicInfo **devices);
    void PrintAllDeviceInfo();
    void PrintConnectedDevicesInfo();

    int SendBytesAdapt(const int sessionId, const std::string &data);
    int SendBytesToAllDevice(const std::string &data);

    int SendStreamAdapt(const int sessionId, const std::string &data);
    int SendStreamToAllDevice(const std::string &data);

    int SendFileAdapt(const int sessionId, const char *fileList[], const uint32_t fileCnt);

    bool IsDeviceConnected(const std::string &deviceId);

    std::string GetNetIdBySessionId(const int sessionId);
    std::string GetLocalNetId();
    std::string GetIpByNetId(const std::string &networkId);

#ifdef ACCESS_TOKEN
    void AddPermission();
#endif // ACCESS_TOKEN

};

#endif  //SOFTBUS_ADAPTER__H