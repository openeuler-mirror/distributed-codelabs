/**********************************************************************
 Copyright (c) 2020-2023, Unitree Robotics.Co.Ltd. All rights reserved.
***********************************************************************/

#include <unistd.h>
#include <ctime>
#include <stdio.h>
#include "softbus_init.h"
#include "softbus_sdk/robot_walk.h"

#define DEFAULT_GAIT_TYPE 1
#define DEFAULT_VELOCITY 0.4f
#define DOING_TIME_SEC 1

using namespace UNITREE_LEGGED_SDK;

void Custom::UDPRecv()
{
    udp.Recv();
}

void Custom::UDPSend()
{
    udp.Send();
}

void Custom::log(const char *__restrict __format, ...) {
    char str[256];
    sprintf(str, __format);
    time_t now = time(0);
    this->logfile << ctime(&now) << str << "-----" << endl;
}

void Custom::initStandCmd() {
    cmd.mode = 1; // 0:idle, default stand      1:forced stand     2:walk continuously
    cmd.gaitType = 0;
    cmd.speedLevel = 0;
    cmd.footRaiseHeight = 0;
    cmd.bodyHeight = 0;
    cmd.euler[0] = 0;
    cmd.euler[1] = 0;
    cmd.euler[2] = 0;
    cmd.velocity[0] = 0.0f;
    cmd.velocity[1] = 0.0f;
    cmd.yawSpeed = 0.0f;
    cmd.reserve = 0;
}

void Custom::walking() {
    cmd.mode = 2; // 0:idle, default stand      1:forced stand     2:walk continuously
    cmd.gaitType = 0;
    cmd.speedLevel = 0;
    cmd.footRaiseHeight = 0;
    cmd.bodyHeight = 0;
    cmd.euler[0] = 0;
    cmd.euler[1] = 0;
    cmd.euler[2] = 0;
    cmd.velocity[0] = 0.0f;
    cmd.velocity[1] = 0.0f;
    cmd.yawSpeed = 0.0f;
    cmd.reserve = 0;
    sleep(DOING_TIME_SEC);
}

void Custom::RobotControl()
{
    motiontime += 2;
    udp.GetRecv(state);

    if (motiontime > 0 && motiontime < 500)
    {
        this->initStandCmd();
    }

    if (motiontime > 1000 && motiontime < 2000)
    {
        cmd.mode = 6;
    }
    udp.SetSend(cmd);
}

void Custom::dogStartTest()
{
    logger("0: STAND_UP\n");
    logger("7: WALK_YAW_LEFT\n");
    logger("9: WALK_YAW_RIGHT\n");
    logger("8: WALK_FORWARD\n");
    logger("2: WALK_BACKWARD\n");
    logger("4: WALK_LEFT\n");
    logger("6: WALK_RIGHT\n");
    ;
    int type;
    while (1) {
        std::cin >> type;
        logger( "do dog action : %d\n", type);
        this->dogDoAction(type);
        logger( "end dog action : %d\n", type);
    }
}

void Custom::RobotYawLeft() {
    cmd.mode = 2;
    cmd.gaitType = DEFAULT_GAIT_TYPE;
    cmd.footRaiseHeight = 0.05;
    cmd.yawSpeed = -0.8;
    sleep(DOING_TIME_SEC);
    this->walking();
    this->initStandCmd();
    return;
}

void Custom::RobotYawRight() {
    cmd.mode = 2;
    cmd.gaitType = DEFAULT_GAIT_TYPE;
    cmd.footRaiseHeight = 0.05;
    cmd.yawSpeed = 0.8;
    sleep(DOING_TIME_SEC);
    this->walking();
    this->initStandCmd();
    return;
}

void Custom::RobotForward() {
    cmd.mode = 2;
    cmd.gaitType = DEFAULT_GAIT_TYPE;
    cmd.footRaiseHeight = 0.05;
    cmd.velocity[0] = DEFAULT_VELOCITY; // -1  ~ +1
    sleep(DOING_TIME_SEC);
    this->walking();
    this->initStandCmd();
    return;
}

void Custom::RobotLeft() {
    cmd.mode = 2;
    cmd.gaitType = DEFAULT_GAIT_TYPE;
    cmd.footRaiseHeight = 0.05;
    cmd.velocity[1] = DEFAULT_VELOCITY;
    sleep(DOING_TIME_SEC);
    this->walking();
    this->initStandCmd();
    return;
}

// velocity，左正右负
void Custom::RobotRight() {
    cmd.mode = 2;
    cmd.gaitType = DEFAULT_GAIT_TYPE;
    cmd.footRaiseHeight = 0.05;
    cmd.velocity[1] = -DEFAULT_VELOCITY;
    sleep(DOING_TIME_SEC);
    this->walking();
    this->initStandCmd();
    return;
}

void Custom::RobotBackward() {
    cmd.mode = 2;
    cmd.gaitType = DEFAULT_GAIT_TYPE;
    cmd.footRaiseHeight = 0.05;
    cmd.velocity[0] = -DEFAULT_VELOCITY; // -1  ~ +1
    sleep(DOING_TIME_SEC);
    this->walking();
    this->initStandCmd();
    return;
}

void Custom::RobotStandUp() {
    cmd.mode = 6;
    sleep(DOING_TIME_SEC);
    return;
}

void Custom::RobotSitDown() {
    cmd.mode = 5;
    sleep(DOING_TIME_SEC);
    return;
}

void Custom::dogDoActionWithSoftbus(string key)
{
    if (this->actionMap.find(key) == actionMap.end()) {
        logger("key %s error, not exsit.\n", key);
        return;
    }
    logger("do dog action : %s", key);
    this->dogDoAction(this->actionMap[key]);
    logger("end : %s", key);
}

void Custom::dogDoAction(int type)
{
    switch (type)
    {
    case WALK_YAW_LEFT:
        this->RobotYawLeft();
        break;
    case WALK_YAW_RIGHT:
        this->RobotYawRight();
        break;
    case WALK_FORWARD:
        this->RobotForward();
        break;
    case WALK_BACKWARD:
        this->RobotBackward();
        break;
    case WALK_LEFT:
        this->RobotLeft();
        break;
    case WALK_RIGHT:
        this->RobotRight();
        break;
    case STAND_UP:
        this->RobotStandUp();
        break;
    case SIT_DOWN:
        this->RobotSitDown();
        break;
    default:
        return;
    }
}

