/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOMCONTEXTLOG_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOMCONTEXTLOG_H_

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace daemon {

enum LogID {
    Started = 1,
    Stopped,
    VPMerror,
    ClientConnected,
    ClientDisconnected,
    ProcessingSendRequest,
    ClientSubscribed,
    ClientUnsubscribed,
    ReceivingCommand,
    OpenUARTDevice,
    OpenSocketDevice,
    Request,
    Response,
    CancelRequest,

    ReceiveFrameBegin,
    ReceiveFrameEnd,
    SendFrameBegin,
    SendFrameEnd,
    LockDevice,
    Enquiry,
    WaitForACK,
    ByteReceived,
    Timeout,
    FrameSent,
    SendFrameFailed,
    UnlockDevice,
    WaitT5,
    Retrying,
    SendNAK,
    WaitT6,
    ProcessNextFrame,
    SendDone,
    Error,
    WaitForControlCode,
    WaitForENQ,
    ReceiveEnquiryENQ,
    ReceiveEnquiryNAK,
    ReceiveEnquiryByte,
    SendACK,
    StartReceiveFrame,
    ReceivedExtLen,
    FrameLength,
    Denying,
    ReceiveCommand,
    ExtendedLength,
    ReceiveFrameNumber,
    FrameReceived,
    ChecksumOK,
    ChecksumDoesNotMatch,
    ReceiveDone,
    ReceiveProcessNextFrame,
};

enum ErrorLogID {
    RecieveCommunicationErrorRecovery = 1,
    SendCommunicationErrorRecovery,
    SendMaxRetryCountRecovery,
    sendReenquiry_writeError,
    sendReenquiry_readError,
    sendAcknowledgement_ackConflict_Busy,
    sendAcknowledgement_ackConflict_Wait,
    sendAcknowledgement_ackConflict_Deny,
    sendAcknowledgement_timeout,
    sendAcknowledgement_readError,
    sendFrame_writeError,
    sendNak_writeError,

    sendRetry_Error,

    recvChecksum_Mismatch,
    recvChecksum_Timeout,
    recvChecksum_Error,

    recvEnquiry_Nak,
    recvEnquiry_Deny,
    recvEnquiry_Timeout,
    recvEnquiry_Error,

    recvControlCode_Deny,
    recvControlCode_Timeout,
    recvControlCode_Error,

    recvPoll_Error,

    recvLength_Deny,
    recvLength_Timeout,
    recvLength_Error,

    recvDataCommand_Timeout,
    recvDataCommand_Error,

    recvDataExtLen_Timeout,
    recvDataExtLen_Error,

    recvDataFrameNumber_Timeout,
    recvDataFrameNumber_Error,

    recvData_Timeout,
    recvData_Error,

    recvRetry_Error,

    recvNak,
    recvError,
};

void InitializeCpuComLogMessages();
void TerminateCpuComLogMessages();

}  // namespace daemon
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOMCONTEXTLOG_H_
