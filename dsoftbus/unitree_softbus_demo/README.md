# v3.3.1
The unitree_legged_sdk is mainly used for communication between PC and Controller board.
It also can be used in other PCs with UDP.

## Notice
support robot: Aliengo, A1(sport_mode >= v1.19)
support OS: openEuler-22.03-LTS-SP1

**not support robot**: Laikago, Go1.

## Dependencies
* [Boost](http://www.boost.org) (version 1.5.4 or higher)
* [CMake](http://www.cmake.org) (version 2.8.3 or higher)
* [LCM](https://lcm-proj.github.io) (version 1.4.0 or higher)

## Prepare

### Install dependecies
```bash
dnf install glib2-devel cmake make gcc g++ boost boost-devel dsoftbus libboundscheck
```

### Build LCM
```bash
cd lcm-x.x.x
mkdir build
cd build
cmake ../
make
sudo make install
```

## Build Demo
```bash
mkdir build
cd build
cmake ../
make
```

## Usage
Run examples with 'sudo' for memory locking.
```bash
sudo ./robot_client
```

error in openEuler
- error while loading shared libraries: liblcm.so.1: cannot open shared object file: No such file or directory
```bash
sudo ln -s  /usr/local/lib/liblcm.so.1 /lib64/liblcm.so.1
```

## More about the softbus
https://docs.openeuler.org/zh/docs/22.03_LTS_SP1/docs/Distributed/%E5%9F%BA%E4%BA%8E%E5%88%86%E5%B8%83%E5%BC%8F%E8%BD%AF%E6%80%BB%E7%BA%BF%E6%89%A9%E5%B1%95%E7%94%9F%E6%80%81%E4%BA%92%E8%81%94.html
