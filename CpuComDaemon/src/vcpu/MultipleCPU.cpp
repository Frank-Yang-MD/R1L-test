/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "MultipleCPU.h"
#include "CPUCommon.h"
#include "Protocol.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

MultipleCPU::MultipleCPU(std::unique_ptr<Protocol> protocolReceive,
                         std::unique_ptr<Protocol> protocolTransmit,
                         uint8_t address)
    : mProtocolReceive{std::move(protocolReceive)}
    , mProtocolTransmit{std::move(protocolTransmit)}
    , m_sendCodebit(getSendCodebit(address))
    , m_receiveCodebit(getReceiveCodebit(address))
{
}

bool MultipleCPU::initialize() { return true; }

bool MultipleCPU::read(std::pair<common::CpuCommand, std::vector<uint8_t>>& value)
{
    std::vector<uint8_t> data;
    bool received = mProtocolReceive->receive(data);
    if (received) {
        auto unpacked = unpack(data);
        if (std::get<1>(unpacked) == m_receiveCodebit) {
            value = std::make_pair(std::get<0>(unpacked), std::get<2>(unpacked));
        }
        else {
            received = false;
        }
    }
    return received;
}

bool MultipleCPU::write(const common::CpuCommand& command, const std::vector<uint8_t>& data)
{
    return mProtocolTransmit->send(pack(command, m_sendCodebit, data));
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
