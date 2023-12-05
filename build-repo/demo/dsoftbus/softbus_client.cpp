#include <iostream>
#include <sstream>

#include "softbus_adapter.h"

const std::string TARGET_SESSION_NAME = "session_test";

int main(int argc, char const *argv[])
{
    SoftbusAdapter adpter;
    adpter.Init();

    std::string input;
    while (std::getline(std::cin, input)) {
        std::string cmd;
        std::istringstream iss(input);
        iss >> cmd;
        if (cmd == "openAll" || cmd == "openA") {
            adpter.OpenSessionWithAllDevice();
        } else if (cmd == "open") {
            std::string peerNetId;
            iss >> peerNetId;
            adpter.OpenSessionAdapt(peerNetId.c_str(), TARGET_SESSION_NAME.c_str(), TYPE_BYTES);
        } else if (cmd == "openStream" || cmd == "openS") {
            std::string peerNetId;
            iss >> peerNetId;
            adpter.OpenSessionAdapt(peerNetId.c_str(), TARGET_SESSION_NAME.c_str(), TYPE_STREAM);
        } else if (cmd == "openFile" || cmd == "openF") {
            std::string peerNetId;
            iss >> peerNetId;
            adpter.OpenSessionAdapt(peerNetId.c_str(), TARGET_SESSION_NAME.c_str(), TYPE_FILE);
        } else if (cmd == "close") {
            std::string peerNetId;
            iss >> peerNetId;
            adpter.CloseSessionWithNetId(peerNetId);
        } else if (cmd == "sendAll" || cmd == "sendA") {
            std::string data;
            std::getline(iss, data, '"');
            std::getline(iss, data, '"');
            adpter.SendBytesToAllDevice(data);
        } else if (cmd == "send") {
            int sessionId;
            std::string data;
            iss >> sessionId;
            std::getline(iss, data, '"');
            std::getline(iss, data, '"');
            adpter.SendBytesAdapt(sessionId, data);
        } else if (cmd == "sendS" || cmd == "sendStream") {
            int sessionId;
            std::string data;
            iss >> sessionId;
            std::getline(iss, data, '"');
            std::getline(iss, data, '"');
            adpter.SendStreamAdapt(sessionId, data);
        } else if (cmd == "sendFile" || cmd == "sendF") {
            int sessionId;
            iss >> sessionId;
            std::string file;
            std::getline(iss, file, '"');
            std::getline(iss, file, '"');
            const char *fileList[] = {file.c_str()};
            adpter.SendFileAdapt(sessionId, fileList, 1);
        } else if (cmd == "sendAllS" || cmd == "sendAS") {
            std::string data;
            std::getline(iss, data, '"');
            std::getline(iss, data, '"');
            adpter.SendStreamToAllDevice(data);
        } else if (cmd == "allDevices" || cmd == "aD") {
            adpter.PrintAllDeviceInfo();
        } else if (cmd == "conDevices") {
            adpter.PrintConnectedDevicesInfo();
        } else if (cmd == "localNetId") {
            std::cout << adpter.GetLocalNetId() << std::endl;
        } else if (cmd == "netId") {
            int sessionId;
            iss >> sessionId;
            std::cout << adpter.GetNetIdBySessionId(sessionId) << std::endl;
        } else if (cmd == "ip") {
            std::string netId;
            iss >> netId;
            std::cout << adpter.GetIpByNetId(netId) << std::endl;
        } else if (cmd == "quit" || cmd == "q") {
            break;
        } else {
            std::cout << "invalid input, retry." << std::endl;
        }
    }

    adpter.Exit();
    return 0;
}
