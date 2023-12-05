# commonlibrary/c_utils<a name="EN-US_TOPIC_0000001148676553"></a>

## Introduction<a name="section11660541593"></a>

The  **commonlibrary/c_utils**  repository provides the following commonly used C++ utility classes for standard system:

-   Enhanced APIs for operations related to files, paths, and strings
-   APIs related to the read-write lock, semaphore, timer, thread, and thread pool
-   APIs related to the security data container and data serialization
-   Error codes for each subsystem

## Directory Structure<a name="section17271017133915"></a>

```
commonlibrary/c_utils
└─ base
    ├── include       # Header files of APIs open to other subsystems
    ├── src           # Source files
    └── test          # Test code
```
## Condition
Suitable for standard system.

## Build
### Build Component
```
./build.sh --product-name rk3568 --build-target c_utils
```

### Build Shared Library
```
./build.sh --product-name rk3568 --build-target commonlibrary/c_utils/base:utils
```

### Build Static Library
```
./build.sh --product-name rk3568 --build-target commonlibrary/c_utils/base:utilsbase
```
## Coding Directions
### ashmem
```
sptr<Ashmem> ashmem = Ashmem::CreateAshmem(MEMORY_NAME.c_str(), MEMORY_SIZE);
if (ashmem != nullptr) {
    bool ret = ashmem->MapAshmem(PROT_READ | PROT_WRITE);
}

...

// Do not forget to unmap & close ashmem at the end
ashmem->UnmapAshmem();
ashmem->CloseAshmem();
```

### parcel
```
// Write data into parcel in some order at writing port
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
    // Deal with writing failure
}

result = parcel.WriteInt8(data.int8test);
if (!result) {
    // Deal with writing failure
}

result = parcel.WriteInt16(data.int16test);
if (!result) {
    // Deal with writing failure
}

result = parcel.WriteInt32(data.int32test);
if (!result) {
    // Deal with writing failure
}

result = parcel.WriteUint8(data.uint8test);
if (!result) {
    // Deal with writing failure
}

result = parcel.WriteUint16(data.uint16test);
if (!result) {
    // Deal with writing failure
}

result = parcel.WriteUint32(data.uint32test);
if (!result) {
    // Deal with writing failure
}
```
```
// Read data with the order writing in at reading port
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
1. Move this repository from utils/native to commonlibrary/c_utils.
2. Switch component name from utils_base to c_utils.
3. Securec is not in this repository any more. Please use [third_party_bounds_checking_function](https://gitee.com/openharmony/third_party_bounds_checking_function).
## Repositories Involved<a name="section1249817110914"></a>

**[commonlibrary\_c\_utils](https://gitee.com/openharmony/commonlibrary_c_utils)**

[commonlibrary\_utils\_lite](https://gitee.com/openharmony/commonlibrary_utils_lite)

