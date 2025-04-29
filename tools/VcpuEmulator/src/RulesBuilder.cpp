/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "RulesBuilder.h"

#include "Action.h"
#include "Actions.h"
#include "CpuCommand.h"
#include "Events.h"
#include "ICPU.h"
#include "Log.h"
#include "Rule.h"
#include "Rules.h"

#include "Utils.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

using com::mitsubishielectric::ahu::common::CpuCommand;
using com::mitsubishielectric::ahu::common::MLOGD_SERIAL;

RulesBuilder::RulesBuilder(std::shared_ptr<ICPU> mcpu, std::shared_ptr<ICPU> vcpu)
    : IRulesBuilder()
    , m_mcpu(mcpu)
    , m_vcpu(vcpu)
    , m_currentCPU(nullptr)
{
}

RulesBuilder::~RulesBuilder() {}

std::unique_ptr<Rule> RulesBuilder::createRule(const Json::Value& jsonObject)
{
    std::unique_ptr<Rule> rule = nullptr;
    if (!jsonObject["type"].isString()) {
        MLOGD_SERIAL("[emulator]", "Can not create rule, type is not specified or invalid");
    }
    else {
        std::string type(jsonObject["type"].asString());
        if (type == "receive") {
            if (!jsonObject["command"].isString()) {
                MLOGD_SERIAL("[emulator]", "Can not create receive rule, invalid command");
            }
            else {
                auto command = jsonObject["command"].asString();
                rule = std::make_unique<RuleReceive>(command);
                MLOGD_SERIAL("[emulator]", "New rule created for \"%s\"", command.c_str());
            }
        }
        else {
            MLOGD_SERIAL("[emulator]", "[RulesBuilder::CreateRule] Not supported type rule");
        }
    }
    return rule;
}

std::unique_ptr<Action> RulesBuilder::createAction(const Json::Value& jsonObject)
{
    std::unique_ptr<Action> action = nullptr;
    if (!jsonObject["type"].isString()) {
        MLOGD_SERIAL("[emulator]", "Can not create action, type is not specified or invalid");
    }
    else {
        std::string type(jsonObject["type"].asString());
        if (type == "send") {
            CpuCommand command;
            std::vector<uint8_t> data;
            bool isValidCommand =
                utils::commandFromString(jsonObject["command"].asString(), command);
            bool isValidData = utils::dataFromString(jsonObject["data"].asString(), data);
            if (isValidCommand && isValidData) {
                action = std::make_unique<ActionSend>(command, data, m_currentCPU);
                MLOGD_SERIAL("[emulator]", "Created new send action");
            }
            else {
                MLOGD_SERIAL("[emulator]", "Can not create send action");
            }
        }
        else if (type == "print") {
            action = std::make_unique<ActionPrint>(jsonObject["format"].asString());
            MLOGD_SERIAL("[emulator]", "Created new print action");
        }
        else if (type == "resend") {
            auto cpu = (m_currentCPU == m_vcpu) ? m_mcpu : m_vcpu;
            action = std::make_unique<ActionResend>(cpu);
            MLOGD_SERIAL("[emulator]", "Created new resend action");
        }
        else if (type == "replace") {
            CpuCommand command;
            std::vector<uint8_t> data;
            bool isValidCommand =
                utils::commandFromString(jsonObject["command"].asString(), command);
            bool isValidData = utils::dataFromString(jsonObject["data"].asString(), data);
            if (isValidCommand && isValidData) {
                auto cpu = (m_currentCPU == m_vcpu) ? m_mcpu : m_vcpu;
                action = std::make_unique<ActionReplace>(command, data, cpu);
                MLOGD_SERIAL("[emulator]", "Created new replace action");
            }
            else {
                MLOGD_SERIAL("[emulator]", "Can not create replace action");
            }
        }
        else if (type == "nop") {
            action = std::make_unique<ActionNop>();
            MLOGD_SERIAL("[emulator]", "Created new nop action");
        }
    }
    return action;
}

std::unique_ptr<InputEvent> RulesBuilder::createInputEvent(const common::CpuCommand& cpuCommand,
                                                           const std::vector<uint8_t>& data)
{
    return std::make_unique<InputEvent>(cpuCommand, data);
}

void RulesBuilder::setCurrentCPU(std::shared_ptr<ICPU> cpu) { m_currentCPU = cpu; }

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
