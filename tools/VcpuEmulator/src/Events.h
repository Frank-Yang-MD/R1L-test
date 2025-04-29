/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_EVENTS_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_EVENTS_H_

#include <cstdint>
#include <string>
#include <vector>

#include "CpuCommand.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class InputEvent {
public:
    InputEvent(common::CpuCommand command, std::vector<uint8_t> data);
    ~InputEvent() = default;

public:
    common::CpuCommand getCommand() const;
    const std::vector<uint8_t>& getData() const;

private:
    common::CpuCommand m_command;
    std::vector<uint8_t> m_data;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_EVENTS_H_
