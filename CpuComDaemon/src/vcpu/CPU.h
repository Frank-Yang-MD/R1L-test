/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPU_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPU_H_

#include "ICPU.h"

#include <cstdint>
#include <memory>
#include <tuple>
#include <vector>

#include "CpuCommand.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class Protocol;
class CPU : public ICPU {
public:
    explicit CPU(std::unique_ptr<Protocol> protocol, uint8_t address);

public:
    bool initialize() override;
    bool read(std::pair<common::CpuCommand, std::vector<uint8_t>>& value) override;
    bool write(const common::CpuCommand& command, const std::vector<uint8_t>& data) override;

private:
    std::unique_ptr<Protocol> m_protocol;
    const uint8_t m_sendCodebit;
    const uint8_t m_receiveCodebit;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPU_H_
