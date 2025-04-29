project(cpucomdaemon CXX)

file(GLOB_RECURSE CPUCD_SRC "${CPUC_ROOT}/CpuComDaemon/src/*.cpp")
list(REMOVE_ITEM CPUCD_SRC "${CPUC_ROOT}/CpuComDaemon/src/main.cpp")

add_library(CPUCD ${CPUCD_SRC})
SET_TARGET_PROPERTIES(CPUCD PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(CPUCD PUBLIC
    ${CPUC_ROOT}/CpuComDaemon/include/
    ${CPUC_ROOT}/CpuComDaemon/src/
    ${CPUC_ROOT}/CpuComDaemon/src/vcpu/
    ${CPUC_ROOT}/CpuComDaemon/src/vcpu/protocol/
    ${CPUC_ROOT}/CpuComDaemon/src/vcpu/device/
)

target_link_libraries(CPUCD log vehicleconfig)
