/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IMPL_MULTIPLECPU_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IMPL_MULTIPLECPU_H_

#include "ICPU.h"

#include <memory>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class Protocol;

/**
 * @brief The MultipleCPU class impelents new communication protocol
 * where using separete UART interfaces for receive and send messages.
 */
class MultipleCPU : public ICPU {  // LCOV_EXCL_LINE - exclude compiler specific generated stuff
public:
    explicit MultipleCPU(std::unique_ptr<Protocol> protocolReceive,
                         std::unique_ptr<Protocol> protocolTransmit,
                         uint8_t address);
    // ICPU interface
public:
    bool initialize() override;
    bool read(std::pair<common::CpuCommand, std::vector<uint8_t>>& value) override;
    bool write(const common::CpuCommand& command, const std::vector<uint8_t>& data) override;

private:
    std::unique_ptr<Protocol> mProtocolReceive;
    std::unique_ptr<Protocol> mProtocolTransmit;
    const uint8_t m_sendCodebit;
    const uint8_t m_receiveCodebit;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IMPL_MULTIPLECPU_H_
