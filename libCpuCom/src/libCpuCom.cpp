/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "libCpuCom.h"
#include "Log.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {

using common::DisplayTypeDecInt32;
using common::DisplayTypeHexUInt8;
using common::DisplayTypeString;

void InitializeLibCpuComLogMessages()
{
    // clang-format off
    const common::LogMessageFormats logMessages = {
        {LogID::ConnectedToDaemon,                "Connected to CpuComDaemon with fd = %d\n", {DisplayTypeDecInt32("FileDescriptor")}},
        {LogID::CouldNotConnectToDaemon,          "Could not connect to CpuComDaemon's socket(%s)\n", {DisplayTypeString(6, "Socket")}},
        {LogID::WrongCommandResultMessageSize,    "Wrong command result message size\n"},
        {LogID::WrongNotificationMessageSize,     "Wrong notification message size\n"},
        {LogID::Disconnected,                     "Disconnected from daemon\n"},
        {LogID::ConnectionClosed,                 "Connection with daemon has been closed unexpectedly\n"},
        {LogID::NotifyClientConnectionClosed,     "Call onConnectionClosed callback\n"},
        {LogID::ResubscribeAndResume,             "Resubscribe and resume\n"},
        {LogID::Request,                          "Request            %s\n", {DisplayTypeString(36, "Request ID")}},
        {LogID::RequestSent,                      "Send request       %s\n", {DisplayTypeString(36, "Request ID")}},
        {LogID::Response,                         "Response for       %s\n", {DisplayTypeString(36, "Request ID")}},
        {LogID::ResponseDropped,                  "Response dropped   %s\n", {DisplayTypeString(36, "Request ID")}},
        {LogID::ResponseDelivered,                "Response delivered %s\n", {DisplayTypeString(36, "Request ID")}},
        {LogID::CancelRequest,                    "Cancel request     %s\n", {DisplayTypeString(36, "Request ID")}},
        {LogID::SendWithDeliveryConfirmation,     "Send [%02x,%02x] and confirm delivery  id = %s\n", {DisplayTypeHexUInt8("Command"), DisplayTypeHexUInt8("Subcommand"), DisplayTypeString(36, "Request ID")}},
        {LogID::DeliveryStatusProvided,           "Delivery status provided to the caller id = %s\n", {DisplayTypeString(36, "Request ID")}},
        {LogID::DeliveryStatusDropped,            "Delivery status dropped id =                %s\n", {DisplayTypeString(36, "Request ID")}},
        {LogID::Send,                             "Send        [%02x,%02x]\n", {DisplayTypeHexUInt8("Command"), DisplayTypeHexUInt8("Subcommand")}},
        {LogID::Receive,                          "Receive     [%02x,%02x]\n", {DisplayTypeHexUInt8("Command"), DisplayTypeHexUInt8("Subcommand")}},
        {LogID::SendFailed,                       "Send failed [%02x,%02x]\n", {DisplayTypeHexUInt8("Command"), DisplayTypeHexUInt8("Subcommand")}},
        {LogID::NotificationsThreadFunctionPoolHup,     "NotificationsThreadFunction: Received POOLHUP\n"},
        {LogID::NotificationsThreadFunctionPoolHupStopFd, "NotificationsThreadFunction: Received POOLHUP. StopFd\n"},
        {LogID::NotificationsThreadFunctionStopRequest, "NotificationsThreadFunction: Stop request\n"},

    };
    // clang-format on
    common::InitializeFunctionID(common::FunctionID::cpuc_lib, logMessages);
}

void TerminateLibCpuComLogMessages() { common::TerminateFunctionID(common::FunctionID::cpuc_lib); }

}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com