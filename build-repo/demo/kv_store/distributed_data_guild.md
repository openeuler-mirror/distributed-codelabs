# 分布式数据

整体说明:

## 安装

可以使用 [dnf 安装](#dnf-安装) 或者 [软件包离线安装](#软件包离线安装) 中任何一种方式安装:

### dnf 安装

目前分布式已在 [`openEuler-22.03-LTS-SP2 update` 版本的 `EPOL` 仓](https://repo.openeuler.org/openEuler-22.03-LTS-SP2/EPOL/update/main/aarch64/Packages/)中引入，如果是相应 openEuler 版本或者添加了相关软件仓，可以直接使用 dnf 安装

```shell
dnf install distributeddatamgr_kv_store distributeddatamgr_datamgr_service distributeddatamgr_relational_store distributeddatamgr_data_object
```

### 软件包离线安装

目前分布式数据 4 个软件包可在以下仓库下载，选择其一下载即可（注意下载对应系统架构 aarch64 或者 x86 的软件包）：

* [openEuler-22.03-LTS-SP2 update 版本 EPOL 官方仓库](https://repo.openeuler.org/openEuler-22.03-LTS-SP2/EPOL/update/main/aarch64/Packages)
* [EulerMaker openEuler-22.03-LTS-SP2 EPOL 构建工程](https://eulermaker.compass-ci.openeuler.openatom.cn/project/overview?osProject=openEuler-22.03-LTS-SP2:epol)
* [华为云镜像 openEuler-22.03-LTS-SP2 EPOL 软件仓](https://repo.huaweicloud.com/openeuler/openEuler-22.03-LTS-SP2/EPOL/update/main/aarch64/Packages/)

假设 4 个软件包均下载到 `path/to/file/` 下，进入 `path/to/file` 目录后，通过 dnf 命令安装

```shell
dnf install -y *.rpm
```

## 运行服务

安装软件包之后，只是将可执行文件和库文件放到了系统对应目录下，要使用服务需启动服务：

1. 服务启动依赖 `libsec_shared.z.so` 这个动态库，而在 openEuler 下这个动态库为 `libboundscheck.so`（由 `libboundscheck` 软件包提供），因此需要在 `/usr/lib64` 下软链接出一个 `libsec_shared.z.so`：
    ```shell
    ln -s /usr/lib64/libboundscheck.so /usr/lib64/libsec_shared.z.so
    ```

1. 新建demo运行需要的/data目录。因为OpenHarmony下app运行有指定的目录，需要在openEuler下先新增
    ```shell
    mkdir -p /data/app/el0/0/database/com.example.distributed.rdb/rdb
    mkdir -p /data/app/el1/0/database/distributeddata/kvdb
    mkdir -p /data/service/el1/public/database/distributeddata/meta
    ```

1. 启动binder功能，参考[communication_ipc仓库说明](https://gitee.com/src-openeuler/communication_ipc/tree/openEuler-22.03-LTS-SP2/)开启binder功能。

1. 启动分布式数据服务datamgr_service及依赖的服务：samgr/huks_service/softbus_server/device_manager等

    使用我们提供的[服务启动脚本](https://gitee.com/heppen/distributed-data-files/blob/master/scripts/start_services.sh)一键启动这些服务
    ```shell
    ./start_services.sh all # 启动所有服务
    ./start_services.sh samgr # 启动samgr
    ```

1. 暂停所有服务或者某个服务

    使用我们提供的[服务停止脚本](https://gitee.com/heppen/distributed-data-files/blob/master/scripts/stop_services.sh)，可以停止某个服务和所有服务
    ```shell
    ./stop_services.sh all # 停止所有服务
    ./stop_services.sh datamgr # 停止datamgr一个服务
    ```

## 编写客户端应用

1. 编写三个数据库 kv_store / data_object / relational_store 的客户端程序，使用分布式数据功能。

    客户端程序编写可参考[仓库](https://gitee.com/heppen/distributed-data-files)每个数据库的demo源代码，demo在这个仓库每个数据库同名目录下的demo目录下，如`kv_store/demo`

    **注意：demo指定的数据db路径等，请最好保持和所给示例demo的路径移植，否则可能会存在路径不存在或者权限不足问题。**

## 常见问题

* 启动各个服务日志一直在报错 `Binder Driver died`。

    原因：说明系统未开启 binder，可以查看 `/dev/binder` 文件是否存在，如果不存在则说明未开启 binder。

    解决方法：启动binder功能，参考[communication_ipc仓库说明](https://gitee.com/src-openeuler/communication_ipc/blob/46d83ed1462e521ce356aec48ef980dbf84cff80/README.md)开启binder功能。

* `softbus_server` 服务未成功起来，报错 `GetNetworkIfIp ifName:eth0 fail`。

    原因：使用命令 `ip a` 查看当前系统的网卡名称，查看是否有 `eth0` 有线网卡名。 因为 `softbus_server` 是通过 `eth0` 这个有线网卡名来获取 ip 等信息，如果没有 `eth0` 网卡则无法启动 `softbus_server`。

    解决方法一：修改网卡名称为 `eth0`。

    解决方法二：修改 `softbus_server` 源码，将依赖的有线网卡名称改成当前系统的网卡名。

## 参考

1. [分布式文件系统概述](https://docs.openeuler.org/zh/docs/22.03_LTS_SP2/docs/Distributed/%E5%88%86%E5%B8%83%E5%BC%8F%E6%96%87%E4%BB%B6%E7%B3%BB%E7%BB%9F%E6%9E%B6%E6%9E%84.html)