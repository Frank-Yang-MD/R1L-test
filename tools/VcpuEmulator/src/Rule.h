/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_RULE_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_RULE_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "CpuCommand.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class Action;
class InputEvent;

class Rule {
public:
    virtual ~Rule() = default;

public:
    virtual bool satisfy(std::shared_ptr<InputEvent> inputEvent);
    const std::vector<std::shared_ptr<Action>>& actions() const;
    std::vector<std::shared_ptr<Action>>& actions();

private:
    std::vector<std::shared_ptr<Action>> m_actions;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_RULE_H_
