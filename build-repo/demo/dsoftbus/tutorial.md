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

### 代码目录结构

参见：

### 接口列表

参见：

## 虚拟化测试

## 常见问题



## 分布式中间件Sig相关软件仓