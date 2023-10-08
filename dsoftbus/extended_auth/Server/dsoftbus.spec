%define debug_package %{nil}
%global source_dsoftbus_standard dsoftbus_standard
%global release_dsoftbus_standard v3.1.2-release
%global source_embedded_ipc embedded-ipc
%global release_embedded_ipc v1.0.0-release
%global source_commonlibrary_c_utils commonlibrary_c_utils
%global source_security_device_auth security_device_auth
%global source_security_huks security_huks
%global source_third_party_libcoap third_party_libcoap
%global source_third_party_mbedtls third_party_mbedtls
%global source_build build
%global release_build OpenHarmony-v3.0.2-LTS
%global openHarmony_source_release OpenHarmony-v3.1.2-Release

Name: dsoftbus
Version: 1.0.0
Release: 3
Summary: openEuler embedded softbus capability support
License: Apache License 2.0
Url: https://openeuler.gitee.io/yocto-meta-openeuler/features/distributed_softbus.html
Source0: https://gitee.com/openeuler/%{source_dsoftbus_standard}/repository/archive/%{release_dsoftbus_standard}.tar.gz #/%{source_dsoftbus_standard}-%{release_dsoftbus_standard}.tar.gz
Source1: https://gitee.com/openeuler/%{source_embedded_ipc}/repository/archive/%{release_embedded_ipc}.tar.gz #/%{source_embedded_ipc}-%{release_embedded_ipc}.tar.gz
Source2: https://gitee.com/openharmony/%{source_commonlibrary_c_utils}/repository/archive/%{openHarmony_source_release}.tar.gz #/%{source_commonlibrary_c_utils}-%{openHarmony_source_release}.tar.gz
Source3: https://gitee.com/openharmony/%{source_security_device_auth}/repository/archive/%{openHarmony_source_release}.tar.gz #/%{source_security_device_auth}-%{openHarmony_source_release}.tar.gz
Source4: https://gitee.com/openharmony/%{source_security_huks}/repository/archive/%{openHarmony_source_release}.tar.gz #/%{source_security_huks}-%{openHarmony_source_release}.tar.gz
Source5: https://gitee.com/openharmony/%{source_third_party_libcoap}/repository/archive/%{openHarmony_source_release}.tar.gz #/%{source_third_party_libcoap}-%{openHarmony_source_release}.tar.gz
Source6: https://gitee.com/openharmony/%{source_third_party_mbedtls}/repository/archive/%{openHarmony_source_release}.tar.gz #/%{source_third_party_mbedtls}-%{openHarmony_source_release}.tar.gz
Source7: https://gitee.com/openharmony/%{source_build}/repository/archive/%{release_build}.tar.gz #/%{source_build}-%{release_build}.tar.gz

Patch0000: 0000-add-dsoftbus-build-support-for-embedded-env.patch
Patch0001: 0001-support-hichian-for-openeuler.patch
Patch0002: 0002-add-deviceauth-ipc-sdk-compile.patch
Patch0003: 0003-Adaptation-for-dsoftbus.patch
Patch0004: 0004-Adaptation-for-dsoftbus-v3.1.2.patch
Patch0005: 0005-fix-CVE-2021-43666.patch
Patch0006: 0006-fix-CVE-2021-45451.patch
Patch0007: 0007-support-huks-for-openeuler.patch
Patch0008: 0008-support-deviceauth-for-openeuler.patch
Patch0009: 0009-adapter-deviceauth-ipc-service.patch
Patch0010: 0010-simplify-dependency-on-third-party-packages.patch
Patch0011: 0011-change-set-for-obs-build.patch
Patch0012: 0012-Adaptation-for-dsoftbus.patch
Patch0013: 0013-add-productdefine-for-openeuler.patch
Patch0014: 0014-add-depend-for-openeuler.patch
Patch0015: 0015-simplify-dependency-on-third-party-packages.patch
Patch0016: 0016-adapter-cjson-in-openEuler-for-softbus.patch
Patch0017: 0017-simplify-dependency-for-dsoftbus-standard.patch
Patch0018: 0001-001.patch
BuildRequires: gcc, g++, cmake, python, zip, unzip, ninja-build, git, libboundscheck, cjson-devel, openssl-devel, gn

%description
OpenEuler supports distributed softbus capability, which is part of openEuler's embedded capability

%prep
# Create the directories needed for the build
dsoftbus_build_dir="%{_builddir}/dsoftbus_build"
dsoftbus_prebuilts_build_tools="${dsoftbus_build_dir}/prebuilts/build-tools/linux-x86/bin"
dsoftbus_thirdparty="${dsoftbus_build_dir}/third_party"
dsoftbus_utils="${dsoftbus_build_dir}/utils"
dsoftbus_src="${dsoftbus_build_dir}/foundation/communication"
dsoftbus_hichain="${dsoftbus_build_dir}/base/security"
dsoftbus_bounds_checking="${dsoftbus_thirdparty}/bounds_checking_function"
dsoftbus_productdefine="${dsoftbus_build_dir}/productdefine"
dsoftbus_depend="${dsoftbus_build_dir}/depend"

mkdir -p ${dsoftbus_prebuilts_build_tools}
mkdir -p ${dsoftbus_thirdparty}
mkdir -p ${dsoftbus_utils}
mkdir -p ${dsoftbus_src}
mkdir -p ${dsoftbus_hichain}
mkdir -p ${dsoftbus_bounds_checking}
mkdir -p ${dsoftbus_productdefine}
mkdir -p ${dsoftbus_depend}

# Decompressing the compressed package
%setup -q -T -D -b 0 -n %{source_dsoftbus_standard}-%{release_dsoftbus_standard}
%setup -q -T -D -b 1 -n %{source_embedded_ipc}-%{release_embedded_ipc}
%setup -q -T -D -b 2 -n %{source_commonlibrary_c_utils}-%{openHarmony_source_release}
%setup -q -T -D -b 3 -n %{source_security_device_auth}-%{openHarmony_source_release}
%setup -q -T -D -b 4 -n %{source_security_huks}-%{openHarmony_source_release}
%setup -q -T -D -b 5 -n %{source_third_party_libcoap}-%{openHarmony_source_release}
%setup -q -T -D -b 6 -n %{source_third_party_mbedtls}-%{openHarmony_source_release}
%setup -q -T -D -b 7 -n %{source_build}-%{release_build}

cd %{_builddir}
mv %{source_dsoftbus_standard}-%{release_dsoftbus_standard} %{source_dsoftbus_standard}
mv %{source_embedded_ipc}-%{release_embedded_ipc} %{source_embedded_ipc}

#copy gn
gn_dir=$(which gn)
cp ${gn_dir[0]} ${dsoftbus_prebuilts_build_tools}

# copy ninja
ninja_dir=$(which ninja)
cp ${ninja_dir[0]} ${dsoftbus_prebuilts_build_tools}

# unpack third_party
mv %{_builddir}/%{source_build}-%{release_build} ${dsoftbus_build_dir}/build
mkdir %{_builddir}/%{source_build}-%{release_build}
mv %{_builddir}/%{source_third_party_libcoap}-%{openHarmony_source_release} ${dsoftbus_thirdparty}/libcoap
mv %{_builddir}/%{source_third_party_mbedtls}-%{openHarmony_source_release} ${dsoftbus_thirdparty}/mbedtls
mkdir -p ${dsoftbus_thirdparty}/cJSON

# unpack hichain
mv %{_builddir}/%{source_security_huks}-%{openHarmony_source_release} ${dsoftbus_hichain}/huks
mv %{_builddir}/%{source_security_device_auth}-%{openHarmony_source_release} ${dsoftbus_hichain}/deviceauth

# unpack utils
mv %{_builddir}/%{source_commonlibrary_c_utils}-%{openHarmony_source_release} ${dsoftbus_utils}/native

# do patch
%patch -p1 -d ${dsoftbus_build_dir}/build
%patch1 -p1 -d ${dsoftbus_build_dir}/build
%patch2 -p1 -d ${dsoftbus_build_dir}/build 
%patch3 -p1 -d ${dsoftbus_utils}/native
%patch4 -p1 -d ${dsoftbus_thirdparty}/mbedtls
%patch5 -p1 -d ${dsoftbus_thirdparty}/mbedtls
%patch6 -p1 -d ${dsoftbus_thirdparty}/mbedtls
%patch7 -p1 -d ${dsoftbus_hichain}/huks
%patch8 -p1 -d ${dsoftbus_hichain}/deviceauth
%patch9 -p1 -d ${dsoftbus_hichain}/deviceauth
%patch10 -p1 -d ${dsoftbus_hichain}/deviceauth
%patch11 -p1 -d ${dsoftbus_build_dir}/build 
%patch12 -p1 -d ${dsoftbus_thirdparty}/bounds_checking_function
%patch13 -p1 -d ${dsoftbus_productdefine}
%patch14 -p1 -d ${dsoftbus_depend}
%patch15 -p1 -d ${dsoftbus_hichain}/huks
%patch16 -p1 -d ${dsoftbus_thirdparty}/cJSON
%patch17 -p1 -d %{_builddir}/%{source_dsoftbus_standard}
%patch18 -p1 -d ${dsoftbus_build_dir}
# init gn root
ln -s ${dsoftbus_build_dir}/build/build_scripts/build.sh ${dsoftbus_build_dir}/build.sh
ln -s ${dsoftbus_build_dir}/build/core/gn/dotfile.gn ${dsoftbus_build_dir}/.gn

# link selfcode
ln -s %{_builddir}/embedded-ipc ${dsoftbus_build_dir}/depend/ipc
ln -s %{_builddir}/dsoftbus_standard ${dsoftbus_src}/dsoftbus

# link toolchain
toolchain_path="/usr1/openeuler/gcc/openeuler_gcc_arm64le"
ln -s ${toolchain_path} ${dsoftbus_build_dir}/toolchain

# copy libboundscheck file
mkdir ${dsoftbus_thirdparty}/bounds_checking_function/include
cp /usr/include/securec*.h ${dsoftbus_thirdparty}/bounds_checking_function/include/

# link cjson
cjson_devel_include="/usr/include/cjson/cJSON.h"
ln -s ${cjson_devel_include} ${dsoftbus_thirdparty}/cJSON/cJSON.h

%ifarch x86_64
    sed -i 's/"target_cpu": "arm64"/"target_cpu": "x86_64"/' ${dsoftbus_productdefine}/common/device/openEuler.json
%endif

%build
rm -rf %{_builddir}/dsoftbus_build/out
cd %{_builddir}/dsoftbus_build
./build.sh --product-name openEuler

%install
dsoftbus_build_dir="%{_builddir}/dsoftbus_build"
%ifarch x86_64
    dsoftbus_release_dir_name="ohos-x86_64-release"
%endif
%ifarch aarch64
    dsoftbus_release_dir_name="ohos-arm64-release"
%endif

install -d %{buildroot}/%{_includedir}/dsoftbus
install -d %{buildroot}/%{_libdir}
install -d %{buildroot}/%{_bindir}
install -d %{buildroot}/data/data/deviceauth/

# prepare so
install -m 0755 ${dsoftbus_build_dir}/out/${dsoftbus_release_dir_name}/common/common/*.so %{buildroot}/%{_libdir}
install -m 0755 ${dsoftbus_build_dir}/out/${dsoftbus_release_dir_name}/communication/dsoftbus_standard/*.so %{buildroot}/%{_libdir}
install -m 0755 ${dsoftbus_build_dir}/out/${dsoftbus_release_dir_name}/security/huks/*.so %{buildroot}/%{_libdir}
install -m 0755 ${dsoftbus_build_dir}/out/${dsoftbus_release_dir_name}/security/deviceauth_standard/*.so %{buildroot}/%{_libdir}

# prepare bin
install -m 0755 ${dsoftbus_build_dir}/out/${dsoftbus_release_dir_name}/communication/dsoftbus_standard/softbus_server_main %{buildroot}/%{_bindir}

# prepare head files
install -m 554 \
${dsoftbus_build_dir}/foundation/communication/dsoftbus/interfaces/kits/discovery/*.h \
${dsoftbus_build_dir}/foundation/communication/dsoftbus/interfaces/kits/common/*.h \
${dsoftbus_build_dir}/foundation/communication/dsoftbus/interfaces/kits/bus_center/*.h \
${dsoftbus_build_dir}/foundation/communication/dsoftbus/interfaces/kits/transport/*.h \
${dsoftbus_build_dir}/foundation/communication/dsoftbus/core/common/include/softbus_errcode.h \
${dsoftbus_build_dir}/base/security/deviceauth/interfaces/innerkits/*.h \
${dsoftbus_build_dir}/third_party/cJSON/*.h \
${dsoftbus_build_dir}/third_party/bounds_checking_function/include/*.h \
    %{buildroot}/%{_includedir}/dsoftbus

%files
%{_includedir}/dsoftbus/*
%{_bindir}/softbus_server_main
%{_libdir}/*.so
/data/data

%changelog
* Wed Nov 30 2022 liheavy <lihaiwei8@huawei.com> - 1.0.0-3
- Disassemble yocto-embedded-tools and replace cjson, ninja, gn, openssl, libboundscheck
- with openEuler software package 

* Wed Nov 23 2022 xuchongyu <xuchongyu@huawei.com> - 1.0.0-2
- add URL,change branch of libboundscheck to 22.03-LTS-SP1

* Tue Nov 22 2022 xuchongyu <xuchongyu@huawei.com> - 1.0.0-1
- init dsoftbus
