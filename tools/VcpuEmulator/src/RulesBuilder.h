/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_RULESBUILDER_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_RULESBUILDER_H_

#include "IRulesBuilder.h"

#include <utility>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class Action;
class Rule;
class InputEvent;
class VcpuEmulator;
class ICPU;
class RulesBuilder : public IRulesBuilder {
public:
    RulesBuilder(std::shared_ptr<ICPU> mcpu, std::shared_ptr<ICPU> vcpu);
    ~RulesBuilder();

public:
    virtual std::unique_ptr<Rule> createRule(const Json::Value& jsonObject) override;
    virtual std::unique_ptr<Action> createAction(const Json::Value& jsonObject) override;
    virtual std::unique_ptr<InputEvent> createInputEvent(const common::CpuCommand& cpuCommand,
                                                         const std::vector<uint8_t>& data) override;
    virtual void setCurrentCPU(std::shared_ptr<ICPU> cpu) override;

private:
    std::shared_ptr<ICPU> m_mcpu;
    std::shared_ptr<ICPU> m_vcpu;
    std::shared_ptr<ICPU> m_currentCPU;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_RULESBUILDER_H_
