/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "Rule.h"
#include "Action.h"
#include "Events.h"
#include "Log.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

bool Rule::satisfy(std::shared_ptr<InputEvent> /*inputEvent*/) { return false; }

const std::vector<std::shared_ptr<Action>>& Rule::actions() const { return m_actions; }

std::vector<std::shared_ptr<Action>>& Rule::actions() { return m_actions; }

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com