cmake_minimum_required(VERSION 2.8.3)
project(unitree_legged_sdk)

include_directories(include ../OhOeCommunication/OpenHarmonyAppSample/entry/src/main/cpp/include ../openEuler_softbus_client/include)

link_directories(lib /usr/lib64)

add_compile_options(-std=c++11)

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64" )
    set(EXTRA_SOFTBUS_LIBS -pthread libunitree_legged_sdk_amd64.so lcm softbus_client.z boundscheck)
else()
    set(EXTRA_SOFTBUS_LIBS -pthread libunitree_legged_sdk_arm64.so lcm softbus_client.z boundscheck)
endif()

set(CMAKE_CXX_FLAGS "-O3")

#softbus_client
add_executable(robot_client softbus_demo/client/robot_walk.cpp  ../openEuler_softbus_client/src/softbus_init_imp.cpp softbus_demo/client/robot_client.cpp)
target_link_libraries(robot_client ${EXTRA_SOFTBUS_LIBS})
