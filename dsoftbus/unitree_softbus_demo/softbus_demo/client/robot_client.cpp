#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include "securec.h"
#include "discovery_service.h"
#include "softbus_bus_center.h"
#include "session.h"
#include "softbus_init.h"
#include "softbus_sdk/robot_walk.h"

#define PACKAGE_NAME "softbus_sample"
#define LOCAL_SESSION_NAME "Robot_dog"
#define TARGET_SESSION_NAME "hwpad"
#define DEFAULT_CAPABILITY "osdCapability"
#define DEFAULT_SESSION_GROUP "group_test"
#define DEBUG_FLAG_FILE "debug"
#define DEFAULT_PUBLISH_ID 123

static SoftClientInitParam g_param = {
    .package_name = PACKAGE_NAME,
    .local_session_name = LOCAL_SESSION_NAME,
    .target_session_name = TARGET_SESSION_NAME,
    .default_capability = DEFAULT_CAPABILITY,
    .default_session_group = DEFAULT_SESSION_GROUP,
    .default_publish_id = DEFAULT_PUBLISH_ID,
    .opened_sessionId = -1
};

SoftClientInitParam *param = &g_param;

static Custom g_custom(HIGHLEVEL);

int SessionOpened(int sessionId, int result) {
    logger("<SessionOpened>CB: session %d open fail:%d", sessionId, result);
    if (result == 0) {
        g_param.opened_sessionId = sessionId;
    }
    return result;
}

int testDogWithSoftbus()
{
    bool loop = true;
    int ret;
    LoopFunc loop_control("control_loop", g_custom.dt, boost::bind(&Custom::RobotControl, &g_custom));
    LoopFunc loop_udpSend("udp_send", g_custom.dt, 3, boost::bind(&Custom::UDPSend, &g_custom));
    LoopFunc loop_udpRecv("udp_recv", g_custom.dt, 3, boost::bind(&Custom::UDPRecv, &g_custom));

    ret = CreateSessionServerInterface(param, SessionOpened);
    if (ret) {
        logger("CreateSessionServer fail, ret=%d\n", ret);
        return ret;
    }

    ret = PublishServiceInterface(param);
    if (ret) {
        logger("PublishService fail, ret=%d\n", ret);
        goto err_PublishServiceInterface;
    }

    ret = DiscoveryInterface(param);
    if (ret) {
        logger("DiscoveryInterface fail, ret=%d\n", ret);
        goto err_DiscoveryInterface;
    }

    loop_udpSend.start();
    loop_udpRecv.start();
    loop_control.start();

    while (loop) {
        logger("\nInput c to commnuication, Input s to stop:");
        char op = getchar();
        switch(op) {
        case 'c':
            commnunicate(param);
            continue;
        case 's':
            loop = false;
            break;
        case '\n':
            break;
        default:
            continue;
        }
    }

    StopDiscoveryInterface(param);
err_DiscoveryInterface:
    UnPublishServiceInterface(param);
err_PublishServiceInterface:
    RemoveSessionServerInterface(param);
    return 0;
}

int testUnitree()
{
    logger("Communication level is set to HIGH-level.\n");
    logger("WARNING: Make sure the robot is standing on the ground.\n");
    LoopFunc loop_control("control_loop", g_custom.dt, boost::bind(&Custom::RobotControl, &g_custom));
    LoopFunc loop_udpSend("udp_send", g_custom.dt, 3, boost::bind(&Custom::UDPSend, &g_custom));
    LoopFunc loop_udpRecv("udp_recv", g_custom.dt, 3, boost::bind(&Custom::UDPRecv, &g_custom));
    loop_udpSend.start();
    loop_udpRecv.start();
    loop_control.start();
    g_custom.dogStartTest();
    return 0;
}

void debugMod() {
    logger("s to test softbus \nt to test unitree sdk %s");
    char op = getchar();
    switch (op)
    {
    case 's':
        testDogWithSoftbus();
        break;
    case 't':
        testUnitree();
        break;
    case '\n':
        break;
    default:
        break;
    } 
}

bool isDebug() {
    if (!access(DEBUG_FLAG_FILE, F_OK)) {
        return true;
    }
    return false;
}

int main(int argc, char **argv)
{
    if (isDebug()) {
        debugMod();
    } else {
        testDogWithSoftbus();        
    }
}
