#ifndef SOFTBUS_CLIENT_H
#define SOFTBUS_CLIENT_H
#undef LOG_TAG
#define LOG_TAG "SoftBusClient"

#include "softbus_bus_center.h"
#include <string>
#include <map>
typedef void(callBack)(std::string fileName);

const std::string PICTURE_SAVE_PATH = "/data/storage/el2/base/haps/entry/files/";

extern std::map<std::string, int> sessionNameAndId;

struct DeviceDetail {
    std::string deviceName;
    std::string ipaddress;
    int port;
    std::string deviceId;
    std::string networkId;
    int sessionId;
};
/**
 * @brief Creates a session server based on a package name and session name.
 * @return Returns <b>0</b> if the operation is successful; return other numbers when the operation was unsuccessful.
 */
int CreateSessionServerInterface(callBack *func);

/**
 * @brief Remove a session server based on a package name and session name.
 */
void RemoveSessionServerInterface();

/**
 * @brief Close session by session id.
 */
void CloseSessionInterface(int sessionId);

/**
 * @brief Publishes a specified service.
 * @return Returns <b>0</b> if the operation is successful; return other numbers when the operation was unsuccessful.
 */
int PublishServiceInterface();

/**
 * @brief Stop Publishing Services
 */
void UnPublishServiceInterface();

/**
 * @brief Subscribes to a specified service.
 * @return Returns <b>0</b> if the operation is successful; return other numbers when the operation was unsuccessful.
 */
int DiscoveryInterface();

/**
 * @brief Stop subscribes to a specified service.
 */
void StopDiscoveryInterface();

/**
 * @brief Obtains basic information about all the online devices.
 * @return
 */
int GetAllNodeDeviceInfoInterface(NodeBasicInfo **dev);

/**
 * Obtain information about the connected device
 * @param deviceDetail
 * @param dev
 * @param devNum
 * @return 
 */
int GetDeviceDetailInterface(DeviceDetail *deviceDetail, NodeBasicInfo *dev, int devNum);

/**
 *@brief
 */
void FreeNodeInfoInterface(NodeBasicInfo *dev);

/**
 * @brief Initiate a session open request, which is an asynchronous process.
 * @param peerNetworkId
 * @return
 */
int OpenSessionInterface(const char *peerNetworkId, const char *peerSessionName);

/**
 * @brief Sends data based on a session ID.
 * @param sessionId Session id of peer device
 * @param data Data to be sent
 * @param len Length of data to be sent
 * @return Returns <b>0</b> if the operation is successful; return other numbers when the operation was unsuccessful.
 */
int SendBytesInterface(int sessionId, const void *data, unsigned int len);

#endif //SOFTBUS_CLIENT_H