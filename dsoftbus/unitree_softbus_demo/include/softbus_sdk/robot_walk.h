#include "unitree_legged_sdk/unitree_legged_sdk.h"
#include <math.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <map>

using namespace UNITREE_LEGGED_SDK;

#define ALL_TEST 0
#define WALK_TEST 1
#define WALK_YAW_LEFT 7
#define WALK_YAW_RIGHT 9
#define WALK_FORWARD 8
#define WALK_FORWARD 8
#define WALK_LEFT 4
#define WALK_RIGHT 6
#define WALK_BACKWARD 2
#define STAND_UP 0
#define SIT_DOWN 1

class Custom
{
public:
    Custom(uint8_t level) : safe(LeggedType::A1), udp(8090, "192.168.123.161", 8082, sizeof(HighCmd), sizeof(HighState))
    {
        udp.InitCmdData(cmd);
        logfile.open("dog.log");
    }
    void UDPRecv();
    void UDPSend();
    void RobotControl();
    void RobotForward();
    void RobotLeft();
    void RobotRight();
    void RobotBackward();
    void RobotYawLeft();
    void RobotYawRight();
    void RobotStandUp();
    void RobotSitDown();
    void dogStartTest();
    void walking();
    void dogDoActionWithSoftbus(string key);
    void log(const char *__restrict __format, ...);

    Safety safe;
    UDP udp;
    HighCmd cmd = {0};
    HighState state = {0};
    int motiontime = 0;
    float dt = 0.002;     // 0.001~0.01

private:
    void initStandCmd();
    void dogDoAction(int type);

    ofstream logfile;
    std::map<string, int> actionMap = {
        {"forward", 8},
        {"backward", 2},
        {"left", 4},
        {"right", 6},
        {"clockwise", 7},
        {"counterclockwise", 9},
        {"standup", 0},
        {"sitdown", 1}
    };

};
