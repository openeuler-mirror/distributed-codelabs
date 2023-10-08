# 一种基于分布式软总线的设备认证新方法

## 背景

通过引入分布式软总线技术，社区已经实现欧拉与鸿蒙设备、以及欧拉设备之间的互联互通；当前软总线在创建连接的过程中，会调用hichain模块的认证接口，与对端的设备进行认证操作。hichain模块为OpenHarmony提供设备认证能力，支持通过点对点认证方式创建可信群组。本项目实现了一种无pin码认证设备，完成一种自定义认证实现。

## 方法使用

该方法也兼容有pin码的使用

### 客户端

具体两个客户端的代码在Client文件夹当中，在运行客户端程序时候，根据提示输入n选择无pin码方式(具体的device id需要在程序处修改)。

### 服务端

服务端修改程序之后打一个patch，对于patch的使用，在dosftbus.spec当中添加

`Patch0018: 0001-nopincode.patch`

`%patch18 -p1 -d ${dsoftbus_build_dir}`

然后重新安装rpm包。

## 测试

拉起两端设备之后，device1和device2均选择无pin码设备认证方式，测试结果

![](E:\Summer\最终提交\测试截图\无pin码\clientn.jpg)

![servern](E:\Summer\最终提交\测试截图\无pin码\servern.jpg)

![client](E:\Summer\最终提交\测试截图\无pin码\client.png)![server](E:\Summer\最终提交\测试截图\无pin码\server.jpg)

设备认证方法仍旧兼容有pin码认证，测试结果如图

![](E:\Summer\最终提交\测试截图\有pin码\clienty.jpg)

![](E:\Summer\最终提交\测试截图\有pin码\servery.jpg)

![](E:\Summer\最终提交\测试截图\有pin码\client.jpg)

![](E:\Summer\最终提交\测试截图\有pin码\server.jpg)
