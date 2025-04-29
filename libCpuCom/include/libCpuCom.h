/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_LIBCPUCOM_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_LIBCPUCOM_H_

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {

enum LogID {
    ConnectedToDaemon = 1,
    CouldNotConnectToDaemon,
    WrongCommandResultMessageSize,
    WrongNotificationMessageSize,
    Disconnected,
    ConnectionClosed,
    NotifyClientConnectionClosed,
    ResubscribeAndResume,
    Request,
    RequestSent,
    Response,
    ResponseDropped,
    ResponseDelivered,
    CancelRequest,
    SendWithDeliveryConfirmation,
    DeliveryStatusProvided,
    DeliveryStatusDropped,
    Send,
    Receive,
    SendFailed,
    NotificationsThreadFunctionPoolHup,
    NotificationsThreadFunctionPoolHupStopFd,
    NotificationsThreadFunctionStopRequest,
};

void InitializeLibCpuComLogMessages();
void TerminateLibCpuComLogMessages();

}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_LIBCPUCOM_H_
