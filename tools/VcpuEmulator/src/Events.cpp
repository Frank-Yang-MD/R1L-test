#include "Events.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

InputEvent::InputEvent(common::CpuCommand command, std::vector<uint8_t> data)
    : m_command(std::move(command))
    , m_data(std::move(data))
{
}

common::CpuCommand InputEvent::getCommand() const { return m_command; }
const std::vector<uint8_t>& InputEvent::getData() const { return m_data; }

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
