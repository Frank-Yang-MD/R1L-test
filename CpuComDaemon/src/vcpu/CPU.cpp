/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "CPU.h"
#include "CPUCommon.h"

#include "Log.h"

#include "Protocol.h"
#include "socket/MasterDevice.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

CPU::CPU(std::unique_ptr<Protocol> protocol, uint8_t address)
    : m_protocol(std::move(protocol))
    , m_sendCodebit(getSendCodebit(address))
    , m_receiveCodebit(getReceiveCodebit(address))
{
}

bool CPU::initialize() { return true; }

bool CPU::read(std::pair<common::CpuCommand, std::vector<uint8_t>>& value)
{
    std::vector<uint8_t> data;
    bool received = m_protocol->receive(data);
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

bool CPU::write(const common::CpuCommand& command, const std::vector<uint8_t>& data)
{
    return m_protocol->send(pack(command, m_sendCodebit, data));
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
