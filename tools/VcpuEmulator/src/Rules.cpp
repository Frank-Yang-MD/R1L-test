/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "Rules.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "Events.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

using common::CpuCommand;

RuleReceive::RuleReceive(const std::string& pattern)
    : Rule()
    , m_pattern(pattern)
{
}

bool RuleReceive::satisfy(std::shared_ptr<InputEvent> inputEvent)
{
    std::stringstream commandStream;
    commandStream << std::hex << std::setfill('0');
    commandStream << std::setw(2) << static_cast<int>(inputEvent->getCommand().first) << ",";
    commandStream << std::setw(2) << static_cast<int>(inputEvent->getCommand().second);
    return std::regex_match(commandStream.str(), m_pattern);
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
