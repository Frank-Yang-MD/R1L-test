/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCKICPU_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCKICPU_H_

#include "ICPU.h"

#include <gmock/gmock.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class MockICPU : public ICPU {
public:
    MOCK_METHOD0(initialize, bool());
    MOCK_METHOD1(read, bool(std::pair<common::CpuCommand, std::vector<uint8_t>>&));
    MOCK_METHOD2(write, bool(const common::CpuCommand&, const std::vector<uint8_t>&));
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCKICPU_H_
