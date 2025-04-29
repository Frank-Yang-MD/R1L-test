/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "CPUCommon.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

std::vector<uint8_t> pack(common::CpuCommand cpuCommand,
                          uint8_t codeBit,
                          const std::vector<uint8_t>& data)
{
    std::vector<uint8_t> out;
    out.push_back(cpuCommand.first);
    out.push_back(cpuCommand.second);
    out.push_back(codeBit);
    out.insert(out.end(), data.begin(), data.end());
    return out;
}  // LCOV_EXCL_LINE - exclude compiler specific generated stuff

std::tuple<common::CpuCommand, uint8_t, std::vector<uint8_t>> unpack(
    const std::vector<uint8_t>& data)
{
    auto i = data.begin();
    uint8_t command = (*i);
    i = std::next(i);  // COMMAND
    uint8_t subCommand = (*i);
    i = std::next(i);  // SUBCOMMAND
    uint8_t codeBit = (*i);
    i = std::next(i);                         // CODEBIT
    std::vector<uint8_t> out{i, data.end()};  // DATA
    return std::make_tuple(std::make_pair(command, subCommand), codeBit, out);
}

// V-CPU address - 001;
// M-CPU address - 010;
// codebit b7..b5 - sender address;
// codebit b4..b2 - receiver address;
// codebit b1..b0 - frame division flags, 00 - no division
uint8_t getSendCodebit(uint8_t address)
{
    uint8_t receiverAddress = address;
    uint8_t senderAddress = address == kAddressVCPU ? kAddressMCPU : kAddressVCPU;
    return (senderAddress << 5) + (receiverAddress << 2) + 0x00;
}

uint8_t getReceiveCodebit(uint8_t address)
{
    uint8_t senderAddress = address;
    uint8_t receiverAddress = address == kAddressVCPU ? kAddressMCPU : kAddressVCPU;
    return (senderAddress << 5) + (receiverAddress << 2) + 0x00;
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
