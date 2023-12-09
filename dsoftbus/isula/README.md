# 嵌入式3.2版本软总线使用指导

## 背景
在服务器22.03-LTS-SP2版本中软总线已经升级到3.2版本，使用了binder作为IPC底层驱动。在树莓派静默无业务场景下，资源占用由原来的CPU单核80%降低到1%，并且支持上层的分布式数据模块的拓展。此次在嵌入式23.09版本中，利用了嵌入式新支持的isula特性，制作了3.2软总线容器镜像，可以在嵌入式环境中通过几行命令就完成软总线的安装部署，并与嵌入式、服务器设备通信测试。

当前版本中只支持使用树莓派设备，如果有更多设备支持的需求，欢迎在 [分布式软总线](https://gitee.com/openeuler/dsoftbus_standard) 提相关的issue，我们会与你一块参与新设备的开发测试工作。

## 宿主机环境准备

使用嵌入式镜像烧录树莓派，由于需要使用的isula容器在systemd镜像中能力较完善，所以使用的是systemd管理进程的镜像
```
http://121.36.84.172/dailybuild/EBS-openEuler-23.09/openeuler-2023-09-21-14-42-10/embedded_img/aarch64/raspberrypi4-64-systemd/openeuler-image-raspberrypi4-64-20230921165629.rootfs.rpi-sdimg
```

嵌入式设备网络配置
```
https://openeuler.gitee.io/yocto-meta-openeuler/master/linux/network/network_config.html
```

若重启网络报错Failed to restart XXXXXX: Unit is masked.
```shell
systemctl unmask service networking
service networking restart
```

挂载binder驱动，树莓派嵌入式镜像自带了binder，可以直接挂载
```shell
mkdir /dev/binderfs
mount -t binder binder /dev/binderfs
```

由于需要使用binder能力，关闭selinux
```shell
setenforce 0
```

启动isulad服务后台运行
```
isulad &
```

下载软总线镜像
```shell
cd /home; wget http://121.36.84.172/dailybuild/EBS-openEuler-23.09/openeuler-2023-09-22-11-46-02/embedded_img/dsoftbus_isula_image/softbus.xz
```

使用isula加载软总线镜像
```shell
isula import /home/softbus.xz softbus
```

查看加载的镜像ID
```shell
isula images
```

参考查询到的IMAGE ID，以host网络、特权模式启动并进入软总线容器，映射binder路径及软总线端口
```
isula run  -it  --privileged --net=host --name=softbus  -v /dev/binderfs/binder:/dev/binder  -p 5684:5684/udp 2edfcbc18543  bash
```

## 容器中启动并测试软总线

在容器中写SN号，注意此SN号是该设备标识，需要与其他设备不一致，建议使用本机IP
```shell
echo  192.168.8.115  > /etc/SN
```

启动软总线
```shell
/system/bin/start_services.sh all
```

启动容器客户端demo用于测试，客户端也可以参考[分布式软总线](https://gitee.com/openeuler/dsoftbus_standard)编写
```shell
/system/bin/softbus_client
```

打开所有session连接
```
openA
```

发送消息给所有session
```
sendA "hello world"
```

在另一台设备中重复以上操作，若在看到对端client收到了hello world字符串，便说明设备间软总线通信OK

如果想在其他环境使用自己修改后的镜像，可以在**宿主机**执行以下命令导出镜像文件
```
isula export -o [导出镜像保存目录] [镜像名称]
```
