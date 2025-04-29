/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_ACTIONS_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_ACTIONS_H_

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "Action.h"
#include "CpuCommand.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class ICPU;
class ActionSend : public Action {
public:
    explicit ActionSend(common::CpuCommand command,
                        std::vector<uint8_t> data,
                        std::shared_ptr<ICPU> cpu);

public:
    virtual void execute(std::shared_ptr<InputEvent> inputEvent) override;

private:
    common::CpuCommand m_command;
    std::vector<uint8_t> m_data;
    std::shared_ptr<ICPU> m_cpu;
};

class ActionPrint : public Action {
public:
    explicit ActionPrint(std::string format);

public:
    virtual void execute(std::shared_ptr<InputEvent> inputEvent) override;

private:
    std::string m_format;
};

class ActionResend : public Action {
public:
    explicit ActionResend(std::shared_ptr<ICPU> cpu);

public:
    virtual void execute(std::shared_ptr<InputEvent> inputEvent) override;

private:
    std::shared_ptr<ICPU> m_cpu;
};

class ActionReplace : public Action {
public:
    explicit ActionReplace(common::CpuCommand command,
                           std::vector<uint8_t> data,
                           std::shared_ptr<ICPU> cpu);

public:
    virtual void execute(std::shared_ptr<InputEvent> inputEvent) override;

private:
    common::CpuCommand m_command;
    std::vector<uint8_t> m_data;
    std::shared_ptr<ICPU> m_cpu;
};

class ActionNop : public Action {
public:
    explicit ActionNop();

public:
    virtual void execute(std::shared_ptr<InputEvent> inputEvent) override;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_ACTIONS_H_
