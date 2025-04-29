/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_ACTION_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_ACTION_H_

#include <memory>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class InputEvent;

class Action {
public:
    virtual ~Action() = default;
    virtual void execute(std::shared_ptr<InputEvent> inputEvent) = 0;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_ACTION_H_
