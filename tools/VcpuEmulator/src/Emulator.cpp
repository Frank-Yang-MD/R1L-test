/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "Emulator.h"

#include <fstream>
#include <iomanip>
#include <iterator>
#include <json/json.h>
#include <sstream>
#include <thread>
#include <tuple>

#include "Action.h"
#include "Actions.h"
#include "CpuCommand.h"
#include "Events.h"
#include "Log.h"
#include "Protocol.h"
#include "Repeater.h"
#include "Rule.h"
#include "Rules.h"
#include "RulesBuilder.h"
#include "ThreadPool.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

using com::mitsubishielectric::ahu::common::CpuCommand;
using com::mitsubishielectric::ahu::common::MLOGD_SERIAL;

namespace {
const char* const kVCPUEmulatorConfigFile = "/odm/etc/emulator-vcpu.config";
const char* const kMCPUEmulatorConfigFile = "/odm/etc/emulator-mcpu.config";
}  // namespace

Emulator::Emulator(std::unique_ptr<IRulesBuilder> rulesBuilder,
                   std::shared_ptr<ICPU> mcpu,
                   std::shared_ptr<ICPU> vcpu)
    : m_mcpuInputThread(nullptr)
    , m_vcpuInputThread(nullptr)
    , m_workerThread(std::make_unique<common::ThreadPool>())
    , m_mcpu(std::move(mcpu))
    , m_vcpu(std::move(vcpu))
    , m_rulesBuilder(std::move(rulesBuilder))
{
}

Emulator::~Emulator() {}

void Emulator::start()
{
    if (loadCPUConfig(kVCPUEmulatorConfigFile, m_vcpu, m_vcpuRules)) {
        MLOGD_SERIAL("[emulator]", "Load VCPU config - %s", kVCPUEmulatorConfigFile);
    }
    else {
        MLOGD_SERIAL("[emulator]", "Failed to load VCPU config - %s", kVCPUEmulatorConfigFile);
    }

    if (loadCPUConfig(kMCPUEmulatorConfigFile, m_mcpu, m_mcpuRules)) {
        MLOGD_SERIAL("[emulator]", "Load MCPU config - %s", kMCPUEmulatorConfigFile);
    }
    else {
        MLOGD_SERIAL("[emulator]", "Failed to load MCPU config - %s", kMCPUEmulatorConfigFile);
    }

    m_mcpuInputThread = std::make_unique<std::thread>(&Emulator::mcpuThreadFunction, this);
    m_vcpuInputThread = std::make_unique<std::thread>(&Emulator::vcpuThreadFunction, this);
}

void Emulator::send(const CpuCommand& command, const std::vector<uint8_t>& data)
{
    m_workerThread->push(std::bind(&ICPU::write, m_mcpu, command, data));
}

void Emulator::repeat(std::string name,
                      common::CpuCommand command,
                      const std::vector<uint8_t>& data,
                      std::chrono::milliseconds interval)
{
    if (m_repeaters.find(name) == m_repeaters.end()) {
        auto l = [=]() { send(command, data); };
        m_repeaters.emplace(name, std::make_unique<Repeater>(l, interval));
        m_repeaters[name]->start();
    }
}

void Emulator::stoprepeat(const std::string& name)
{
    if (m_repeaters.find(name) != m_repeaters.end()) {
        m_repeaters[name]->stop();
        m_repeaters.erase(name);
    }
}

void Emulator::stoprepeat()
{
    for (auto i = m_repeaters.begin(); i != m_repeaters.end(); ++i) {
        i->second->stop();
    }
    m_repeaters.clear();
}

void Emulator::mcpuThreadFunction()
{
    using namespace std::placeholders;

    while (true) {
        std::pair<CpuCommand, std::vector<uint8_t>> data;
        bool received = m_mcpu->read(data);
        if (received) {
            std::shared_ptr<InputEvent> event(
                m_rulesBuilder->createInputEvent(data.first, data.second));
            for (auto&& rule : m_mcpuRules) {
                if (rule->satisfy(event)) {
                    for (auto&& action : rule->actions()) {
                        m_workerThread->push(std::bind(&Action::execute, action, _1), event);
                    }
                }
            }
            m_workerThread->push(std::bind(&Action::execute, m_defaultActions[m_mcpu], _1), event);
        }
    }
}

void Emulator::vcpuThreadFunction()
{
    using namespace std::placeholders;

    while (true) {
        std::pair<CpuCommand, std::vector<uint8_t>> data;
        bool received = m_vcpu->read(data);
        if (received) {
            std::shared_ptr<InputEvent> event(
                m_rulesBuilder->createInputEvent(data.first, data.second));
            for (auto&& rule : m_vcpuRules) {
                if (rule->satisfy(event)) {
                    for (auto&& action : rule->actions()) {
                        m_workerThread->push(std::bind(&Action::execute, action, _1), event);
                    }
                }
            }
            m_workerThread->push(std::bind(&Action::execute, m_defaultActions[m_vcpu], _1), event);
        }
    }
}

bool Emulator::loadCPUConfig(const char* configFileName,
                             std::shared_ptr<ICPU> cpu,
                             std::vector<std::unique_ptr<Rule>>& rules)
{
    m_rulesBuilder->setCurrentCPU(cpu);
    std::ifstream config(configFileName);
    if (config) {
        Json::Reader reader;
        Json::Value value;
        reader.parse(config, value);

        auto jDefaultAction = value["default-action"];
        std::shared_ptr<Action> defaultAction(m_rulesBuilder->createAction(jDefaultAction));
        m_defaultActions.insert({cpu, defaultAction});

        for (uint32_t i = 0; i < value["rules"].size(); i++) {
            const Json::Value& jRule = value["rules"][i];
            auto rule = m_rulesBuilder->createRule(jRule);
            if (rule) {
                for (unsigned int j = 0; j < jRule["actions"].size(); j++) {
                    const Json::Value& jAction = jRule["actions"][j];
                    auto action = m_rulesBuilder->createAction(jAction);
                    rule->actions().push_back(std::move(action));
                }
                rules.push_back(std::move(rule));
            }
        }
        config.close();
    }
    else {
        return false;
    }
    return true;
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
