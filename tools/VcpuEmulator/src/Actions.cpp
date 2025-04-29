/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "Actions.h"

#include <iomanip>
#include <iterator>
#include <memory>
#include <sstream>
#include <utility>

#include "CpuCommand.h"
#include "Events.h"
#include "ICPU.h"
#include "Log.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

using com::mitsubishielectric::ahu::common::CpuCommand;
using com::mitsubishielectric::ahu::common::MLOGD_SERIAL;

ActionSend::ActionSend(CpuCommand command, std::vector<uint8_t> data, std::shared_ptr<ICPU> cpu)
    : m_command(std::move(command))
    , m_data(std::move(data))
    , m_cpu(std::move(cpu))
{
}

void ActionSend::execute(std::shared_ptr<InputEvent> /*inputEvent*/)
{
    m_cpu->write(m_command, m_data);
}

ActionPrint::ActionPrint(std::string format)
    : m_format(std::move(format))
{
}

void ActionPrint::execute(std::shared_ptr<InputEvent> inputEvent)
{
    const std::vector<uint8_t>& data = inputEvent->getData();
    std::stringstream ss;
    if (!data.empty()) {
        ss << std::setbase(16);
        std::copy(data.begin(), std::prev(data.end()), std::ostream_iterator<int>(ss, ","));
        ss << static_cast<int>(data.back());
    }
    auto command = inputEvent->getCommand();
    MLOGD_SERIAL("[emulator]", m_format.c_str(), command.first, command.second, ss.str().c_str());
}

ActionResend::ActionResend(std::shared_ptr<ICPU> cpu)
    : m_cpu(std::move(cpu))
{
}

void ActionResend::execute(std::shared_ptr<InputEvent> inputEvent)
{
    m_cpu->write(inputEvent->getCommand(), inputEvent->getData());
}

ActionReplace::ActionReplace(CpuCommand command,
                             std::vector<uint8_t> data,
                             std::shared_ptr<ICPU> cpu)
    : m_command(std::move(command))
    , m_data(std::move(data))
    , m_cpu(std::move(cpu))
{
}

void ActionReplace::execute(std::shared_ptr<InputEvent> /*inputEvent*/)
{
    m_cpu->write(m_command, m_data);
}

ActionNop::ActionNop()
    : Action()
{
}

void ActionNop::execute(std::shared_ptr<InputEvent>) {}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
