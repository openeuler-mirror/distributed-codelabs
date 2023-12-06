# 分布式软总线使用说明

## 上游软件仓

[OpenHarmony分布式软总线仓库](https://gitee.com/openharmony/communication_dsoftbus)

## 功能展示

gif图

## 安装使用

### 两个版本
| 软总线版本 | 支持openEuler版本 | 所属软件包 | 所属软件仓 |
| --- | --- | --- | --- |
| 3.1.2 | 22.03-LTS-SP1 / 22.03-LTS-SP2 / 23.03 | dsoftbus | Epol |
| 3.2 | 22.03-LTS-SP2 | distributed-utils | Epol |

### 3.1.2版本

1. 3.1.2软总线版本已集成在22.03-LTS-SP1/23.03/22.03-LTS-SP2/23.09服务器和边缘版本Epol仓库中(注意软件源中开启了Epol源)，可直接通过dnf安装
    ```shell
    dnf install dsoftbus
    ```
1. 启动softbus server服务
    ```shell
    ./softbus_server_main
    ```

#### 3.2版本（推荐）

1. 3.2版本软总线目前集成在22.03-LTS-SP2服务器和边缘版本Epol仓库中(注意软件源中开启了Epol源)，所属软件包是distributed-utils中，可直接通过dnf安装。
    ```shell
    dnf install distributed-utils
    ```
1. 启动softbus server服务。3.2版本软总线引入上游OpenHarmony的System Ability机制，需要启动一些依赖服务，统一使用start_services.sh启动。


### 软件包信息

* 支持系统版本：22.03-LTS-SP2、22.03-LTS-SP1的服务器和边缘版本
* 所属软件仓：EPOL仓
* 当前分布式软总线版本：3.2，对应上游OpenHarmony的3.2-Release版本

### 安装

### 启动软总线服务

## 编写应用

因为openEuler下分布式软总线沿用了OpenHarmony上的System Ability模式，并且dsoftbus软件包（目前在distributed-utils软件包中）提供了服务端的功能，所以要真正使用分布式软总线的能力，需要编写客户端应用。

### 应用示例

可参考：

### 发现发布

#### 基本概念：

* **发现**: 发现有特定能力的设备
* **发布**: 发布设备自己的能力
* **主动**: 进行数据广播
* **被动**: 进行数据监听

举例： 
1. 主动发现会广播数据包去找特定能力的设备，类比生活场景：废品回收骑着车沿街叫卖自己收集某种废品。
1. 被动发布会监听某种广播数据包，类比生活场景：自己家中有某种废品，时刻等待着街上是否有这种废品的回收车。

#### 流程

*注意：`PublishService`/`UnPublishService`/`StartDiscovery`和`StopDiscovery` 四个接口已不推荐使用，推荐使用`PublishLNN`/`StopPublishLNN`/`RefreshLNN`和`StopRefreshLNN`*

1. 发布自身能力
    ```C
    int32_t PublishLNN(const char *pkgName, const PublishInfo *info, const IPublishCb *cb);
    ```
1. 发现某种能力
    ```C
    int32_t RefreshLNN(const char *pkgName, const SubscribeInfo *info, const IRefreshCallback *cb);
    ```
1. 暂停发现发布
    ```C
    int32_t StopPublishLNN(const char *pkgName, int32_t publishId);
    int32_t RefreshLNN(const char *pkgName, const SubscribeInfo *info, const IRefreshCallback *cb);
    ```

### 组网

组网可以分为自组网和手动组网：

* 自组网：在发现设备之后，主动触发组网流程，和发现的设备组网。在目前openEuler版本下使用Coap发现发布的设备会进行自组网。
* 手动组网：发现设备之后，通过叫用JoinLNN等组网接口，和目标设备之间进行组网。

下面介绍手动组网使用流程（Coap发现无需以下手动组网流程）：

#### 手动组网（自组网下无需此流程）

1. 利用对端地址信息，发起组网请求
    ```C
    int32_t JoinLNN(const char *pkgName, ConnectionAddr *target, OnJoinLNNResult cb);
    ```
1. 注册节点监听网络中节点的状态变化
    ```C
    int32_t RegNodeDeviceStateCb(const char *pkgName, INodeStateCb *callback);
    ```
1. 传输完成并且关闭连接后，注销节点状态监听，并且发送退网请求。
    ```C
    int32_t UnregNodeDeviceStateCb(INodeStateCb *callback);
    int32_t LeaveLNN(const char *pkgName, const char *networkId, OnLeaveLNNResult cb);
    ```

### 连接传输

1. 创建会话服务，设置会话相关回调，可在回调中进行消息处理。
    ```C
    int CreateSessionServer(const char *pkgName, const char *sessionName, const ISessionListener* listener);
    ```
1. 创建会话，后续基于会话进行数据传输。
    ```C
    int OpenSession(const char *mySessionName, const char *peerSessionName, const char *peerNetworkId, const char *groupId, const SessionAttribute* attr);
    // attr参数控制会话属性
    typedef struct {
        int dataType; // 指定会话传输的bytes/message/stream/file
        int linkTypeNum;
        LinkType linkType[LINK_TYPE_MAX]; // 指定链接方式WLAN/P2P/BR等
        union {
            struct StreamAttr {
                int streamType; // 指定流传输类型：裸流/视频流/音频流
            } streamAttr;
        } attr;
    } SessionAttribute;
    ```
1. 使用sessionId对应会话向其他设备发送数据
    ```C
    int SendBytes(int sessionId, const void *data, unsigned int len);
    int SendMessage(int sessionId, const void *data, unsigned int len);
    int SendStream(int sessionId, const StreamData *data, const StreamData *ext, const StreamFrameInfo *param);
    ```
1. 关闭会话
    ```C
    void CloseSession(int sessionId);
    ```
1. 删除会话服务
    ```C
    int RemoveSessionServer(const char *pkgName, const char *sessionName);
    ```
#### 文件传输

文件传输在OpenSession之前，可以注册对文件传输的处理回调，在回调中可以根据文件的传输事件进行自定义操作。

```C
    int SetFileSendListener(const char *pkgName, const char *sessionName, const IFileSendListener *sendListener);
    int SetFileReceiveListener(const char *pkgName, const char *sessionName,const IFileReceiveListener *recvListener, const char *rootDir);
```

## client使用指导

以下操作在设备A/B上都需要操作

1. 物理机环境安装openEuler LTS SP2以上版本，环境需要准备好binder驱动 [binder驱动安装指导](https://gitee.com/src-openeuler/communication_ipc/blob/openEuler-22.03-LTS-SP2/README.md "binder")

2. 编译softbus_client
```
bash build-repo/demo/dsoftbus/build.sh
```

3. 安装softbus_server
```
dnf install dsoftbus -y
```

4. 启动softbus_server
```
/system/bin/start_services.sh all
```

#### 物理机单client场景：物理机上server与client一一对应
1. 在物理机A、物理机B上分别启动softbus_client二进制文件
```
./softbus_client
```

2. 在物理机A上打开session
```
openA
```

3. 在A上执行命令发送消息给B
```
sendAll "hello"
```

4. 此时在B上的softbus_client看到字符串"hello"说明消息通信成功

#### 容器多client场景
1. 执行脚本在物理机A打包加载容器镜像，[base镜像链接](https://repo.openeuler.org/openEuler-22.03-LTS-SP2/docker_img/aarch64/openEuler-docker.aarch64.tar.xz "img")
```
bash build-repo/demo/dsoftbus/docker_img_build.sh
```

2. 在物理机A启动容器镜像softbus_client，并将相关SDK、binder驱动映射到容器中
```
docker run  -it  --privileged --net=host --name=softbus  -v /dev/binderfs/binder:/dev/binder  -v  /system:/system  -v /usr/lib64:/usr/lib64  -p 5684:5684/udp softbus_client  bash
```

3. 镜像中写/etc/SI文件，作为该容器中softclient的标志，注意需要和组网中其他client的不一致避免冲突
```
echo 123 > /etc/SI
```

4. 容器中启动softbus_client
```
./home/softbus_client
```

5. 在本节点上可重复步骤1-3启动不同的client

6. 在物理机B启动softbus
```
./build-repo/demo/dsoftbus/softbus_client
```

7. 在设备A中的各个中client分别执行命令打开所有连接
```
openA
```

8. 在设备B中client查看所有已经打开的session
```
conDevices
```
回显说明，已经打开了两个session，id分别是4、3，此sessionid是本机的softbus_server分配的，后续发送消息需要用到该id
```
conDevices
12-11 20:18:58.350 2780335 2780335 I A0fffe/SOFTBUS_DEMO: [SOFTBUS_DEMO]::PrintConnectedDevicesInfo: sessionId:4, networkId: 3c95f61941b81c48ecd73fef881262b82fcbc58e9b1f545e2097b0dc6fecea37
12-11 20:18:58.350 2780335 2780335 I A0fffe/SOFTBUS_DEMO: [SOFTBUS_DEMO]::PrintConnectedDevicesInfo: sessionId:3, networkId: 3c95f61941b81c48ecd73fef881262b82fcbc58e9b1f545e2097b0dc6fecea37
```

9. 设备B发送使用“session 4”消息至设备A的client
```
send 4 "hello4"
12-11 20:19:14.975 2780335 2780335 I C015c0/dsoftbus: [TRAN]SendBytes: sessionId=4
```
观察设备A中哪个client有回显，说明设备B使用session 4与该client通信，同理可以使用session 3发送消息至另一个client

10. 设备A容器中client发送使用消息至设备B的client，查看打开的session id
```
conDevices
12-11 20:25:48.995 344047 344047 I A0fffe/SOFTBUS_DEMO: [SOFTBUS_DEMO]::PrintConnectedDevicesInfo: sessionId:1, networkId: e69eab4e2d657264dfbb2006fdfa15524f4a27edeff0baa26d5d2a2b9502f300
```
说明使用session 1通信

11. 发送消息测试，在设备B中接受到该字段说明消息接收成功
```
send 1 "hello1"
```

12. 设备A物理机上client发送使用消息至设备B的client，查看打开的session id
```
conDevices
12-11 20:37:24.823 3512580 3512580 I A0fffe/SOFTBUS_DEMO: [SOFTBUS_DEMO]::PrintConnectedDevicesInfo: sessionId:1, networkId: e69eab4e2d657264dfbb2006fdfa15524f4a27edeff0baa26d5d2a2b9502f300
```
说明使用session 1通信

13. 发送消息测试，在设备B中接受到该字段说明消息接收成功
```
send 1 "hello1"
```

#### session id 说明

在上个章节的测试中，发现两个client之间通信使用的session id不一样。原因是session id是本机server分配的，只需要在本机范围内互斥。同样容器与虚机也做了session name，group name的隔离，所以本机容器与虚机上client的session id是一致的

