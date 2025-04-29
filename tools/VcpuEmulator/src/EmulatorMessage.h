/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MESSAGE_TYPE_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MESSAGE_TYPE_H_

#include "Message.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

const char* kEmulatorCLISocketName = "emulatorcli";

enum class EmulatorId : uint8_t { Command };
using EmulatorMessage = common::Message<EmulatorId>;
using EmulatorMessageParser = common::Message<EmulatorId>::Parser;

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MESSAGE_TYPE_H_
