/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_COMMON_CPU_COMMAND_H_
#define COM_MITSUBISHIELECTRIC_AHU_COMMON_CPU_COMMAND_H_

#include <cstdint>
#include <utility>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace common {

using CpuCommand = std::pair<uint8_t, uint8_t>;

}  // namespace common
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_COMMON_CPU_COMMAND_H_
