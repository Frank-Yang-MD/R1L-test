/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_EMULATOR_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_EMULATOR_H_

#include <cstdint>
#include <map>
#include <memory>
#include <thread>
#include <vector>

#include "CpuCommand.h"
#include "ICPU.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace common {

class ThreadPool;
}  // namespace common
namespace cpucom {
namespace impl {

class IRulesBuilder;
class Rule;
class Protocol;
class Repeater;
class Action;
class Emulator {
public:
    Emulator(std::unique_ptr<IRulesBuilder> builder,
             std::shared_ptr<ICPU> mcpu,
             std::shared_ptr<ICPU> vcpu);
    ~Emulator();

    void start();
    void send(const common::CpuCommand& command, const std::vector<uint8_t>& data);
    void repeat(std::string name,
                common::CpuCommand command,
                const std::vector<uint8_t>& data,
                std::chrono::milliseconds interval);
    void stoprepeat(const std::string& name);
    void stoprepeat();

private:
    void mcpuThreadFunction();
    void vcpuThreadFunction();

    bool loadCPUConfig(const char* configFileName,
                       std::shared_ptr<ICPU> cpu,
                       std::vector<std::unique_ptr<Rule>>& rules);

private:
    std::unique_ptr<std::thread> m_mcpuInputThread;
    std::unique_ptr<std::thread> m_vcpuInputThread;
    std::unique_ptr<common::ThreadPool> m_workerThread;
    std::shared_ptr<ICPU> m_mcpu;
    std::shared_ptr<ICPU> m_vcpu;
    std::unique_ptr<IRulesBuilder> m_rulesBuilder;
    std::vector<std::unique_ptr<Rule>> m_mcpuRules;
    std::vector<std::unique_ptr<Rule>> m_vcpuRules;
    std::map<std::string, std::unique_ptr<Repeater>> m_repeaters;
    std::map<std::shared_ptr<ICPU>, std::shared_ptr<Action>> m_defaultActions;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_EMULATOR_H_
