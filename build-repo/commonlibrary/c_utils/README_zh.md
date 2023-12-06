# C++公共基础库<a name="ZH-CN_TOPIC_0000001148676553"></a>


## 简介<a name="section11660541593"></a>

C++公共基础类库为标准系统提供了一些常用的C++开发工具类，包括：

-   文件、路径、字符串相关操作的能力增强接口
-   读写锁、信号量、定时器、线程增强及线程池等接口
-   安全数据容器、数据序列化等接口
-   各子系统的错误码相关定义

## 目录<a name="section17271017133915"></a>

```
commonlibrary/c_utils
└─ base
    ├── include       # 对各子系统开放的接口头文件
    ├── src           # 源文件
    └── test          # 测试代码
```

## 约束

适用于标准系统。
## 编译构建
### 编译部件
```
./build.sh --product-name rk3568 --build-target c_utils
```

### 编译动态库
```
./build.sh --product-name rk3568 --build-target commonlibrary/c_utils/base:utils
```

### 编译静态库
```
./build.sh --product-name rk3568 --build-target commonlibrary/c_utils/base:utilsbase
```
## 使用说明
### ashmem
```
sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
if (ashmem != nullptr) {
    bool ret = ashmem->MapAshmem(PROT_READ | PROT_WRITE);
}

...

// 当使用结束时不要忘记解映射和关闭ashmem
ashmem->UnmapAshmem();
ashmem->CloseAshmem();
```

### parcel
```
// 写入端以某种顺序写入数据
struct TestData {
    bool booltest;
    int8_t int8test;
    int16_t int16test;
    int32_t int32test;
    uint8_t uint8test;
    uint16_t uint16test;
    uint32_t uint32test;
};

...

Parcel parcel(nullptr);
struct TestData data = { true, -0x34, 0x5634, -0x12345678, 0x34, 0x5634, 0x12345678 };
bool result = false;

result = parcel.WriteBool(data.booltest);
if (!result) {
    // 写失败处理
}

result = parcel.WriteInt8(data.int8test);
if (!result) {
    // 写失败处理
}

result = parcel.WriteInt16(data.int16test);
if (!result) {
    // 写失败处理
}

result = parcel.WriteInt32(data.int32test);
if (!result) {
    // 写失败处理
}

result = parcel.WriteUint8(data.uint8test);
if (!result) {
    // 写失败处理
}

result = parcel.WriteUint16(data.uint16test);
if (!result) {
    // 写失败处理
}

result = parcel.WriteUint32(data.uint32test);
if (!result) {
    // 写失败处理
}
```
```
// 接收端根据写入端写入顺序读取数据
bool readbool = parcel.ReadBool();

int8_t readint8 = parcel.ReadInt8();

int16_t readint16 = parcel.ReadInt16();

int32_t readint32 = parcel.ReadInt32();

uint8_t readuint8 = parcel.ReadUint8();

uint16_t readuint16 = parcel.ReadUint16();

uint32_t readuint32 = parcel.ReadUint32();
```
### refbase
```
class TestRefBase : public RefBase {
...
};
...
sptr<TestRefBase> test(new TestRefBase());
...
```
### timer
```
void TimeOutCallback()
{
    ...
}
...
Utils::Timer timer("test_timer");
uint32_t ret = timer.Setup();
timer.Register(TimeOutCallback, 1, true);
std::this_thread::sleep_for(std::chrono::milliseconds(15));
timer.Shutdown();
```

## Changelog
**2022/10/10**
1. 路径变更。由utils/native移动至commonlibrary/c_utils；
2. 部件名变更。由utils_base变更为c_utils；
3. 不再提供安全C库能力。请使用[third_party_bounds_checking_function](https://gitee.com/openharmony/third_party_bounds_checking_function)。
## 相关仓<a name="section1249817110914"></a>

**[commonlibrary\_c\_utils](https://gitee.com/openharmony/commonlibrary_c_utils)**

[commonlibrary\_utils\_lite](https://gitee.com/openharmony/commonlibrary_utils_lite)

