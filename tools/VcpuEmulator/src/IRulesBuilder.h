/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IRULESBUILDER_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IRULESBUILDER_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "CpuCommand.h"
#include "json/json.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class Action;
class Rule;
class InputEvent;
class ICPU;
class IRulesBuilder {
public:
    virtual ~IRulesBuilder() = default;

    virtual std::unique_ptr<Rule> createRule(const Json::Value& jsonObject) = 0;
    virtual std::unique_ptr<Action> createAction(const Json::Value& jsonObject) = 0;
    virtual std::unique_ptr<InputEvent> createInputEvent(const common::CpuCommand& command,
                                                         const std::vector<uint8_t>& data) = 0;
    virtual void setCurrentCPU(std::shared_ptr<ICPU> cpu) = 0;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IRULESBUILDER_H_
