/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SEQ_PACKET_SOCKET_SERVER_H
#define SEQ_PACKET_SOCKET_SERVER_H

#include "socket_server.h"

namespace OHOS {
namespace HiviewDFX {
using namespace std::chrono;

class SeqPacketSocketServer : public SocketServer {
public:
    using AcceptingHandler = std::function<void(std::unique_ptr<Socket>)>;
    using TimtoutHandler = std::function<void(void)>;

    SeqPacketSocketServer(const std::string& serverPath, unsigned int maxListenNumber)
        : SocketServer(serverPath, SOCK_SEQPACKET), maxListenNumber(maxListenNumber) {}
    ~SeqPacketSocketServer() = default;
    int StartAcceptingConnection(AcceptingHandler onAccepted, milliseconds ms, TimtoutHandler timeOutFunc);
private:
    unsigned int maxListenNumber;
    int AcceptingLoop(AcceptingHandler func, milliseconds ms, TimtoutHandler timeOutFunc);
};
} // namespace HiviewDFX
} // namespace OHOS
#endif /* SEQ_PACKET_SOCKET_SERVER_H */
