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

const char* const kCpuComDaemonSocketName = "cpucom";

enum class CpuComId : uint8_t {
    SendCommand,
    SendCommandResult,
    Subscribe,
    Unsubscribe,
    Notification,
    SendCommandWithDeliveryStatus,
    DeliveryStatus,
    Request,
    RequestResponse,
    CancelRequest,
};
using CpuComMessage = common::Message<CpuComId>;
using CpuComMessageParser = common::Message<CpuComId>::Parser;

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MESSAGE_TYPE_H_
