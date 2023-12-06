# 分布式文件服务

## **简介**

分布式文件服务提供跨设备的、符合POSIX规范的文件访问能力。其在分布式软总线动态组网的基础上，为网络上各个设备结点提供一个统一的、逻辑的、树形的文件系统层次结构。

<br>**图1**. 分布式文件系统架构图</br>
![架构图](https://images.gitee.com/uploads/images/2022/0107/113135_1cef6c44_7809790.png "hmdfs架构图update20220107.png")


其包括如下几个核心模块:

- distributed_file_daemon：分布式文件管理常驻用户态服务，负责接入设备组网、数据传输能力，并负责挂载hmdfs。
- distributed_file_service：分布式文件访问能力服务，对应用提供分布式扩展能力。
- hmdfs(Harmony Distributed File System)：分布式文件系统核心模块，是一种面向移动分布式场景的、高性能的、基于内核实现的、堆叠式文件系统。

## **目录**

```raw
/foundation/filemanagement/dfs_service
├── frameworks                      // 接口实现
│   └── native                      // c/c++ 接口实现
├── interfaces                      // 接口声明
│   └── inner_API                   // 对内接口声明
├── services                        // 服务实现
│   └── distributed_file_daemon     // daemon常驻服务实现
|       └── include                 // daemon常驻服务头文件,类声明
|       └── src                     // daemon常驻服务源文件
|           └── device              // 设备上下线管理
|           └── ipc                 // daemon进程拉起退出流程以及ipc接口实现
|           └── mount_point         // hmdfs挂载管理
|           └── network             // 软总线和内核会话session交互相关
|       └── test                    // 测试代码
|   └── distributed_file_service    // 对应用提供扩展能力的服务实现
├── utils                           // 公共组件
|    ├── log                        // 日志组件
|    └── system                     // 平台相关组件
```

## **约束**

### 接口支持情况

分布式文件管理当前不支持或有限支持如下 VFS 系统调用：

- symlink：不支持
- mmap：仅支持读
- rename：仅支持同目录操作

### 规格

- 最大目录层级

    与被堆叠文件系统，即data分区所用文件系统，如ext4，f2fs等保持一致。
- 最大文件名长度
 
    取决于680B与被堆叠文件支持长度的最小值。f2fs和ext4均为255B。
- 最大单文件大小

    取决于$2^{64}$B与被堆叠文件系统支持最大单文件大小的最小值。ext4单文件最大为16TB，f2fs单文件最大为3.94TB。

## **说明**

### 使用说明

可以使用终端调试分布式文件管理能力。
-   环境准备

1) 设备A和设备B连接同一局域网
2) 设备A和设备B登录同一华为账号
-   分布式文件系统能力使用
 
1) 经过以上环境准备后，即建立了设备间分布式文件互相访问的能力。
2) 可以在应用内通过```context.distributedFileDir()```接口获取本应用沙箱内的分布式路径。
3) 在此路径下可以像操作本地文件一样,操作远端设备的文件，比如可以进行创建、删除、读写文件和目录。
4) 在一端设备上进行步骤3的文件操作后，在组网的所有分布式设备上都可以查看相应的修改。

## 相关仓

- [分布式软总线-SoftBus](https://gitee.com/openharmony/communication_dsoftbus)
- [分布式硬件-设备管理](https://gitee.com/openharmony/device_manager)
- [用户程序框架](https://gitee.com/openharmony/appexecfwk_standard)
