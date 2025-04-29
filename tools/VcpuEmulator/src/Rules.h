/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_RULES_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_RULES_H_

#include <cstdint>
#include <memory>
#include <regex>
#include <string>
#include <utility>

#include "CpuCommand.h"
#include "Rule.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class InputEvent;
class RuleReceive : public Rule {
public:
    explicit RuleReceive(const std::string& pattern);
    virtual ~RuleReceive() = default;
    virtual bool satisfy(std::shared_ptr<InputEvent> inputEvent) override;

private:
    const std::regex m_pattern;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_RULES_H_
