/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_ICPU_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_ICPU_H_

#include <stdint.h>
#include <tuple>
#include <vector>

#include "CpuCommand.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class ICPU {
public:
    // LCOV_EXCL_START - exclude compiler generated d-tors which are impossible to trigger
    virtual ~ICPU() = default;
    // LCOV_EXCL_STOP

public:
    virtual bool initialize() = 0;
    virtual bool read(std::pair<common::CpuCommand, std::vector<uint8_t>>& value) = 0;
    virtual bool write(const common::CpuCommand& command, const std::vector<uint8_t>& data) = 0;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_ICPU_H_
