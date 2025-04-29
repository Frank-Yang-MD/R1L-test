cmake_minimum_required(VERSION 3.9)
project(CPUCOM)

set(CPUC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(ANDROID_ROOT "${CPUC_ROOT}/../../../..")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

include_directories(
    #
    # Google test include directories
    #
    ${ANDROID_ROOT}/external/googletest/googletest/include
    ${ANDROID_ROOT}/external/googletest/googlemock/include

    #
    # External dependencies
    #
    ${ANDROID_ROOT}/vendor/melco/efw/Common/libMelcoCommon/include
    ${ANDROID_ROOT}/system/core/liblog/include
    ${ANDROID_ROOT}/system/core/libutils/include
    ${ANDROID_ROOT}/external/jsoncpp/include/
    ${ANDROID_ROOT}/vendor/melco/efw/VehicleConf/libVehicleConfig/include
#    "${ANDROID_ROOT}/system/core/libbacktrace/include"
#    "${ANDROID_ROOT}/system/core/libsystem/include"
    ${ANDROID_ROOT}/system/core/libcutils/include
    ${ANDROID_ROOT}/vendor/melco/efw/VehiclePwrMgr/libvehiclepwr/include
    ${ANDROID_ROOT}/vendor/melco/efw/VehiclePwrMgr/common/include
    ${ANDROID_ROOT}/vendor/melco/efw/CpuComm/libCpuCom/include
    ${ANDROID_ROOT}/vendor/melco/efw/LargeDataTrans/liblargedatatrans/include
    ${ANDROID_ROOT}/vendor/melco/efw/Common/libMelcoCommon/include
#    "${ANDROID_ROOT}/external/libcxx/include"
#    "${ANDROID_ROOT}/external/libcxxabi/include"
#    "${ANDROID_ROOT}/bionic/libc/arch-arm64/include"
#    "${ANDROID_ROOT}/bionic/libc/include"
#    "${ANDROID_ROOT}/bionic/libc/kernel/uapi"
#    "${ANDROID_ROOT}/bionic/libc/kernel/uapi/asm-arm64"
#    "${ANDROID_ROOT}/bionic/libc/kernel/android/scsi"
#    "${ANDROID_ROOT}/bionic/libc/kernel/android/uapi"
#    "${ANDROID_ROOT}/system/core/include"
#    "${ANDROID_ROOT}/system/media/audio/include"
#    "${ANDROID_ROOT}/hardware/libhardware/include"
#    "${ANDROID_ROOT}/hardware/libhardware_legacy/include"
#    "${ANDROID_ROOT}/hardware/ril/include"
#    "${ANDROID_ROOT}/libnativehelper/include"
#    "${ANDROID_ROOT}/frameworks/native/include"
#    "${ANDROID_ROOT}/frameworks/native/opengl/include"
#    "${ANDROID_ROOT}/frameworks/av/include"
#    "${ANDROID_ROOT}/libnativehelper/include_deprecated"

    #
    # Internal dependencies
    #
    ${CPUC_ROOT}/Internal/include/
)

link_directories(
    ${ANDROID_ROOT}/out/host/linux-x86/lib64
)

 set(gtest_src ${ANDROID_ROOT}/external/googletest)
 set(gtest_build ${CMAKE_CURRENT_BINARY_DIR}/googletest/)

 set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
 add_subdirectory(${gtest_src} ${gtest_build} )

add_subdirectory(CpuComDaemon/src)
add_subdirectory(CpuComDaemon/test)
# add_subdirectory(libCpuCom/test)
# add_subdirectory(libCpuCom/src)
# add_subdirectory(Internal)
