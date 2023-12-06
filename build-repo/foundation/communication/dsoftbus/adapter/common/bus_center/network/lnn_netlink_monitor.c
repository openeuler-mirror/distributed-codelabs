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

#include "lnn_event_monitor_impl.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef __MUSL__
#define __MUSL__
#endif

#include <arpa/inet.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "bus_center_event.h"
#include "lnn_network_manager.h"
#include "securec.h"
#include "softbus_adapter_errcode.h"
#include "softbus_adapter_mem.h"
#include "softbus_adapter_socket.h"
#include "softbus_adapter_thread.h"
#include "softbus_errcode.h"
#include "softbus_log.h"

#undef NLMSG_OK
#define NLMSG_OK(nlh, len)                                                                               \
    (((len) >= (int32_t)(sizeof(struct nlmsghdr))) && (((nlh)->nlmsg_len) >= sizeof(struct nlmsghdr)) && \
        ((int32_t)((nlh)->nlmsg_len) <= (len)))

#define DEFAULT_NETLINK_RECVBUF (8 * 1024)

static int32_t CreateNetlinkSocket(void)
{
    int32_t sockFd;
    struct sockaddr_nl nladdr;
    int32_t sz = DEFAULT_NETLINK_RECVBUF;

    int32_t ret = SoftBusSocketCreate(SOFTBUS_PF_NETLINK, SOFTBUS_SOCK_DGRAM | SOFTBUS_SOCK_CLOEXEC,
        NETLINK_ROUTE, &sockFd);
    if (ret != SOFTBUS_ADAPTER_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "open netlink socket failed");
        return SOFTBUS_ERR;
    }
    if (SoftBusSocketSetOpt(sockFd, SOFTBUS_SOL_SOCKET, SOFTBUS_SO_RCVBUFFORCE, &sz, sizeof(sz)) < 0 &&
        SoftBusSocketSetOpt(sockFd, SOFTBUS_SOL_SOCKET, SOFTBUS_SO_RCVBUF, &sz, sizeof(sz)) < 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "set uevent socket SO_RCVBUF option failed");
        SoftBusSocketClose(sockFd);
        return SOFTBUS_ERR;
    }
    if (memset_s(&nladdr, sizeof(nladdr), 0, sizeof(nladdr)) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "init sockaddr_nl failed");
        return SOFTBUS_ERR;
    }
    nladdr.nl_family = SOFTBUS_AF_NETLINK;
    // Kernel will assign a unique nl_pid if set to zero.
    nladdr.nl_pid = 0;
    nladdr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;
    if (SoftBusSocketBind(sockFd, (SoftBusSockAddr *)&nladdr, sizeof(nladdr)) < 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "bind netlink socket failed");
        SoftBusSocketClose(sockFd);
        return SOFTBUS_ERR;
    }
    return sockFd;
}

static void ParseRtAttr(struct rtattr **tb, int max, struct rtattr *attr, int len)
{
    struct rtattr *attr1 = attr;
    for (; RTA_OK(attr1, len); attr1 = RTA_NEXT(attr1, len)) {
        if (attr1->rta_type <= max) {
            tb[attr1->rta_type] = attr1;
        }
    }
}

static void ProcessAddrEvent(struct nlmsghdr *nlh)
{
    if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(struct ifaddrmsg))) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "Wrong len");
        return;
    }
    struct ifaddrmsg *ifa = (struct ifaddrmsg *)NLMSG_DATA(nlh);
    LnnNetIfType type = LNN_NETIF_TYPE_ETH;
    char ifnameBuffer[NET_IF_NAME_LEN];
    char *ifName = if_indextoname(ifa->ifa_index, ifnameBuffer);
    if (ifName == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "invalid iface index");
        return;
    }
    if (LnnGetNetIfTypeByName(ifName, &type) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "ProcessAddrEvent LnnGetNetIfTypeByName error");
        return;
    }
    if (type == LNN_NETIF_TYPE_ETH || type == LNN_NETIF_TYPE_WLAN) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "network addr changed, type:%d", type);
        LnnNotifyAddressChangedEvent(ifName);
    }
}

static void ProcessLinkEvent(struct nlmsghdr *nlh)
{
    int len;
    struct rtattr *tb[IFLA_MAX + 1] = {NULL};
    struct ifinfomsg *ifinfo = NLMSG_DATA(nlh);
    LnnNetIfType type = LNN_NETIF_TYPE_ETH;

    len = (int32_t)nlh->nlmsg_len - NLMSG_SPACE(sizeof(*ifinfo));
    ParseRtAttr(tb, IFLA_MAX, IFLA_RTA(ifinfo), len);

    if (tb[IFLA_IFNAME] == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "netlink msg is invalid");
        return;
    }

    if (LnnGetNetIfTypeByName(RTA_DATA(tb[IFLA_IFNAME]), &type) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "ProcessAddrEvent LnnGetNetIfTypeByName error");
        return;
    }
    if (type == LNN_NETIF_TYPE_ETH || type == LNN_NETIF_TYPE_WLAN) {
        SoftBusLog(
            SOFTBUS_LOG_LNN, SOFTBUS_LOG_WARN, "%s:link status changed, type:%d", RTA_DATA(tb[IFLA_IFNAME]), type);
        LnnNotifyAddressChangedEvent(RTA_DATA(tb[IFLA_IFNAME]));
    }
}

static void *NetlinkMonitorThread(void *para)
{
    int32_t sockFd;
    int32_t len;
    struct nlmsghdr *nlh = NULL;

    (void)para;
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "netlink monitor thread start");
    sockFd = CreateNetlinkSocket();
    if (sockFd < 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "create netlink socket failed");
        return NULL;
    }
    uint8_t *buffer = (uint8_t *)SoftBusCalloc(DEFAULT_NETLINK_RECVBUF * sizeof(uint8_t));
    if (buffer == NULL) {
        SoftBusSocketClose(sockFd);
        return NULL;
    }
    while (true) {
        (void)memset_s(buffer, DEFAULT_NETLINK_RECVBUF, 0, DEFAULT_NETLINK_RECVBUF);
        len = SoftBusSocketRecv(sockFd, buffer, DEFAULT_NETLINK_RECVBUF, 0);
        if (len < 0 && len == SOFTBUS_ADAPTER_SOCKET_EINTR) {
            continue;
        }
        if (len < 0) {
            SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "recv netlink socket error");
            break;
        }
        if (len < (int32_t)sizeof(struct nlmsghdr)) {
            SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "recv buffer not enough");
            continue;
        }
        nlh = (struct nlmsghdr *)buffer;
        while (NLMSG_OK(nlh, len) && nlh->nlmsg_type != NLMSG_DONE) {
            switch (nlh->nlmsg_type) {
                case RTM_NEWADDR:
                case RTM_DELADDR:
                    ProcessAddrEvent(nlh);
                    break;
                case RTM_NEWLINK:
                case RTM_DELLINK:
                    ProcessLinkEvent(nlh);
                    break;
                default:
                    break;
            }
            nlh = NLMSG_NEXT(nlh, len);
        }
    }
    SoftBusSocketClose(sockFd);
    SoftBusFree(buffer);
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "netlink monitor thread exit");
    return NULL;
}

int32_t LnnInitNetlinkMonitorImpl(void)
{
    SoftBusThread tid;
    if (SoftBusThreadCreate(&tid, NULL, NetlinkMonitorThread, NULL) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "create ip change monitor thread failed");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}