/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IMPL_CPUCOMMON_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IMPL_CPUCOMMON_H_

#include "CpuCommand.h"

#include <tuple>
#include <vector>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

constexpr uint8_t kAddressVCPU = 0x01;
constexpr uint8_t kAddressMCPU = 0x02;

std::vector<uint8_t> pack(common::CpuCommand cpuCommand,
                          uint8_t codeBit,
                          const std::vector<uint8_t>& data);

std::tuple<common::CpuCommand, uint8_t, std::vector<uint8_t>> unpack(
    const std::vector<uint8_t>& data);

uint8_t getSendCodebit(uint8_t address);
uint8_t getReceiveCodebit(uint8_t address);

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IMPL_CPUCOMMON_H_
