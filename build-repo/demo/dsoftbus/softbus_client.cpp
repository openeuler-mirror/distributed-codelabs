#include <iostream>
#include <sstream>

#include "softbus_adapter.h"

const std::string TARGET_SESSION_NAME = "session_test";

void printUsage() {
    std::string usage = "\
\n\
\n\
Usage:\
\n\
\n\
usage                                 usage\n\
openAll/openA                         打开所有设备连接\n\
open <netId>                          打开指定设备连接\n\
openWS <netId> <sessionName>          打开指定session连接\n\
openStream/openS <netId>              打开指定设备流连接\n\
openFile/openF <netId>                打开指定设备文件连接\n\
close <netId>                         关闭指定设备连接\n\
sendAll/sendA <str>                   向所有已打开连接的设备发送字符串消息\n\
sendAllS/sendAS <str>                 向所有已打开连接的设备发送流消息\n\
send <sessionId> <str>                向指定sessionID发送字符串消息\n\
sendStream/sendS <sessionId> <str>    向指定sessionID发送流消息\n\
sendFile/sendF <sessionId> <file>     向指定sessionID发送文件\n\
allDevices/aD                         查看局域网中所有设备信息\n\
conDevices                            查看局域网中所有已连接的设备信息\n\
localNetId                            查看本机net id\n\
netId <sessionId>                     查看打开了sessionid设备的netID\n\
ip <netId>                            查看指定设备(netId)的IP\n\
quit                                  本机退出组网\
\n\
\n\
";
    std::cout << usage << std::endl;
}

int main(int argc, char const *argv[])
{
    SoftbusAdapter adpter;
    adpter.Init();
    printUsage();
    std::string input;
    while (std::getline(std::cin, input)) {
        std::string cmd;
        std::istringstream iss(input);
        iss >> cmd;
        if (cmd == "openAll" || cmd == "openA") {
            adpter.OpenSessionWithAllDevice();
        } else if (cmd == "usage") {
            printUsage();
        } else if (cmd == "openWithSessionName") {
            std::string peerNetId;
            iss >> peerNetId;
            std::string sessionName;
            iss >> sessionName;
            adpter.OpenSessionAdapt(peerNetId.c_str(), sessionName.c_str(), TYPE_BYTES);
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
