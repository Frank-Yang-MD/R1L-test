/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "CpuComDaemonLog.h"

#include "Log.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace daemon {

using common::DisplayTypeBool;
using common::DisplayTypeDecInt32;
using common::DisplayTypeDecUInt64;
using common::DisplayTypeHexUInt8;
using common::DisplayTypeString;

void InitializeCpuComLogMessages()
{
    // clang-format off
    const common::LogMessageFormats cpuComDaemonLogMessages =
    {
        {LogID::Started,                    "Started\n"},
        {LogID::Stopped,                    "Stopped\n"},
        {LogID::VPMerror,                   "VPM returned nullptr to ICpuComClient\n"},
        {LogID::ClientConnected,            "Client connected - %d\n", {DisplayTypeDecInt32("FileDescriptor")}},
        {LogID::ClientDisconnected,         "Client disconnected - %d\n", {DisplayTypeDecInt32("FileDescriptor")}},
        {LogID::ProcessingSendRequest,      "Processing send command request: [%02x,%02x]\n", {DisplayTypeHexUInt8("Command"), DisplayTypeHexUInt8("Subcommand")}},
        {LogID::ClientSubscribed,           "Subscribed %d to [%02x,%02x]\n", {DisplayTypeDecInt32("FileDescriptor"), DisplayTypeHexUInt8("Command"), DisplayTypeHexUInt8("Subcommand")}},
        {LogID::ClientUnsubscribed,         "Unsubscribed %d from [%02x,%02x]\n", {DisplayTypeDecInt32("FileDescriptor"), DisplayTypeHexUInt8("Command"), DisplayTypeHexUInt8("Subcommand")}},
        {LogID::ReceivingCommand,           "Receiving [%02x,%02x]. data size = %lu. dispatching...\n", {DisplayTypeHexUInt8("Command"), DisplayTypeHexUInt8("Subcommand"), DisplayTypeDecUInt64("Size")}},
        {LogID::OpenUARTDevice,             "Open UART device: %s\n", {DisplayTypeString(7, "Result")}},
        {LogID::OpenSocketDevice,           "Open Socket device: %s\n", {DisplayTypeString(7, "Result")}},
        {LogID::Request,                    "Request received: %s\n", {DisplayTypeString(36, "Request ID")}},
        {LogID::Response,                   "Respond to request: %s\n", {DisplayTypeString(36, "Request ID")}},
        {LogID::CancelRequest,              "Cancel request: %s\n", {DisplayTypeString(36, "Request ID")}},

        {LogID::ReceiveFrameBegin,          "<RECV "},
        {LogID::ReceiveFrameEnd,            "RECV>\n"},
        {LogID::SendFrameBegin,             "<SEND[%02x,%02x] ", {DisplayTypeHexUInt8("Command"), DisplayTypeHexUInt8("Subcommand")}},
        {LogID::SendFrameEnd,               "SEND>\n"},
        {LogID::LockDevice,                 "lock device -> "},
        {LogID::Enquiry,                    "ENQ -> "},
        {LogID::WaitForACK,                 "wait for ACK(06) -> "},
        {LogID::ByteReceived,               "received %02x -> ", {DisplayTypeHexUInt8("Data")}},
        {LogID::Timeout,                    "timeout -> "},
        {LogID::FrameSent,                  "[frame] -> "},
        {LogID::SendFrameFailed,            "failed to send frame -> "},
        {LogID::UnlockDevice,               "unlock device -> "},
        {LogID::WaitT5,                     "wait T5 -> "},
        {LogID::Retrying,                   "retrying -> "},
        {LogID::SendNAK,                    "NAK -> "},
        {LogID::WaitT6,                     "wait T6 -> "},
        {LogID::ProcessNextFrame,           "done. process next frame -> "},
        {LogID::SendDone,                   "done -> "},
        {LogID::Error,                      "error -> "},
        {LogID::WaitForControlCode,         "wait for %s(%02x) -> ", {DisplayTypeString(3, "Name of code"), DisplayTypeHexUInt8("Code")}},
        {LogID::WaitForENQ,                 "wait for ENQ(05) -> "},
        {LogID::ReceiveEnquiryENQ,          "received %d byte(s) of data. the last one is ENQ -> ", {DisplayTypeDecInt32("Length")}},
        {LogID::ReceiveEnquiryNAK,          "received %d byte(s) of data. the last one is NAK -> ", {DisplayTypeDecInt32("Length")}},
        {LogID::ReceiveEnquiryByte,         "received %d byte(s) of data. the last one is %02x -> ", {DisplayTypeDecInt32("Length"), DisplayTypeHexUInt8("Last data")}},
        {LogID::SendACK,                    "send ACK -> "},
        {LogID::StartReceiveFrame,          "start receive frame -> "},
        {LogID::ReceivedExtLen,             "received EXT_LEN -> "},
        {LogID::FrameLength,                "frame length - %d bytes -> ", {DisplayTypeDecInt32("Length")}},
        {LogID::Denying,                    "received %02x, denying -> ", {DisplayTypeHexUInt8("Length")}},
        {LogID::ReceiveCommand,             "command - [%02x,%02x] -> ", {DisplayTypeHexUInt8("Command"), DisplayTypeHexUInt8("Subcommand")}},
        {LogID::ExtendedLength,             "extended length - %d bytes -> ", {DisplayTypeDecInt32("Length")}},
        {LogID::ReceiveFrameNumber,         "current frame - %d, frames total - %d -> ", {DisplayTypeDecInt32("Current frame"), DisplayTypeDecInt32("Total frame")}},
        {LogID::FrameReceived,              "frame received -> "},
        {LogID::ChecksumOK,                 "checksum ok(%02x) -> ", {DisplayTypeHexUInt8("Checksum")}},
        {LogID::ChecksumDoesNotMatch,       "checksum does not match. expected - %02x, received - %02x -> ", {DisplayTypeHexUInt8("Expected"), common::DisplayTypeHexUInt8("Received")}},
        {LogID::ReceiveDone,                "done -> "},
        {LogID::ReceiveProcessNextFrame,    "done. process next frame -> "},
    };
    const common::LogMessageFormats cpuComDaemonLogErrorMessages =
    {
        {ErrorLogID::RecieveCommunicationErrorRecovery, "recieve communication error"},
        {ErrorLogID::SendCommunicationErrorRecovery, "send communication error"},
        {ErrorLogID::SendMaxRetryCountRecovery, "retry count == %d, reopening UART device: result = %d", {DisplayTypeDecInt32("MaxRetryCount"), DisplayTypeBool("Result")}},

        {ErrorLogID::sendReenquiry_writeError, "sendReenquiry_writeError"},
        {ErrorLogID::sendReenquiry_readError, "sendReenquiry_readError"},
        {ErrorLogID::sendAcknowledgement_ackConflict_Busy, "sendAcknowledgement_ackConflict_Busy"},
        {ErrorLogID::sendAcknowledgement_ackConflict_Wait, "sendAcknowledgement_ackConflict_Wait"},
        {ErrorLogID::sendAcknowledgement_ackConflict_Deny, "sendAcknowledgement_ackConflict_Deny: data %02x", {DisplayTypeHexUInt8("expected")}},
        {ErrorLogID::sendAcknowledgement_timeout, "sendAcknowledgement_timeout"},
        {ErrorLogID::sendAcknowledgement_readError, "sendAcknowledgement_readError"},
        {ErrorLogID::sendFrame_writeError, "sendFrame_writeError"},
        {ErrorLogID::sendNak_writeError, "sendNak_writeError"},

        {ErrorLogID::sendRetry_Error, "sendRetry_Error"},

        {ErrorLogID::recvChecksum_Mismatch, "recvChecksum_Mismatch: expected %02x, received %02x", {DisplayTypeHexUInt8("Expected"), common::DisplayTypeHexUInt8("Received")}},
        {ErrorLogID::recvChecksum_Timeout, "recvChecksum_Timeout"},
        {ErrorLogID::recvChecksum_Error, "recvChecksum_Error"},

        {ErrorLogID::recvEnquiry_Nak, "recvEnquiry_Nak: bytesRead %d", {DisplayTypeDecInt32("MaxRetryCount")}},
        {ErrorLogID::recvEnquiry_Deny, "recvEnquiry_Deny: bytesRead %d, value %02x ", {DisplayTypeDecInt32("MaxRetryCount"), DisplayTypeHexUInt8("value")}},
        {ErrorLogID::recvEnquiry_Timeout, "recvEnquiry_Timeout"},
        {ErrorLogID::recvEnquiry_Error, "recvEnquiry_Error"},

        {ErrorLogID::recvControlCode_Deny, "recvControlCode_Deny: expected %02x, received %02x", {DisplayTypeHexUInt8("expected"), DisplayTypeHexUInt8("received")}},
        {ErrorLogID::recvControlCode_Timeout, "recvControlCode_Timeout"},
        {ErrorLogID::recvControlCode_Error, "recvControlCode_Error"},

        {ErrorLogID::recvPoll_Error, "recvPoll_Error"},

        {ErrorLogID::recvLength_Deny, "recvLength_Deny: frameLength %02x", {DisplayTypeHexUInt8("frameLength")}},
        {ErrorLogID::recvLength_Timeout, "recvLength_Timeout"},
        {ErrorLogID::recvLength_Error, "recvLength_Error"},

        {ErrorLogID::recvDataCommand_Timeout, "recvDataCommand_Timeout"},
        {ErrorLogID::recvDataCommand_Error, "recvDataCommand_Error"},

        {ErrorLogID::recvDataExtLen_Timeout, "recvDataExtLen_Timeout"},
        {ErrorLogID::recvDataExtLen_Error, "recvDataExtLen_Error"},

        {ErrorLogID::recvDataFrameNumber_Timeout, "recvDataFrameNumber_Timeout"},
        {ErrorLogID::recvDataFrameNumber_Error, "recvDataFrameNumber_Error"},

        {ErrorLogID::recvData_Timeout, "recvData_Timeout"},
        {ErrorLogID::recvData_Error, "recvData_Error"},

        {ErrorLogID::recvRetry_Error, "recvRetry_Error"},

        {ErrorLogID::recvNak, "recvNak"},
        {ErrorLogID::recvError, "recvError"},

    };

    // clang-format on

    common::InitializeFunctionID(common::FunctionID::cpuc_daemon, cpuComDaemonLogMessages);
    common::InitializeFunctionID(common::FunctionID::cpuc_daemon_error,
                                 cpuComDaemonLogErrorMessages);
}
void TerminateCpuComLogMessages()
{
    common::TerminateFunctionID(common::FunctionID::cpuc_daemon);
    common::TerminateFunctionID(common::FunctionID::cpuc_daemon_error);
}

}  // namespace daemon
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
