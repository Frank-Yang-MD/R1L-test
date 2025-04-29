/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IMESSAGESERVER_H
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IMESSAGESERVER_H

#include "CpuComMessage.h"
#include "CpuCommand.h"
#include "Error.h"
#include "UUID.h"

#include <messenger/MessageServer.h>
#include <vector>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {

class CpuComDaemon;

namespace impl {

class IMessageServer {
public:
    using MessageServer = common::MessageServer<CpuComId>;
    using SessionID = MessageServer::SessionID;
    using OnStartedHandler = MessageServer::OnStartedHandler;
    using OnStoppedHandler = MessageServer::OnStoppedHandler;
    using OnNewConnectionHandler = MessageServer::OnNewConnectionHandler;
    using OnConnectionClosedHandler = MessageServer::OnConnectionClosedHandler;

    // LCOV_EXCL_START
    virtual ~IMessageServer() = default;
    // LCOV_EXCL_STOP

    virtual bool initialize(OnNewConnectionHandler onNewConnectionHandler,
                            OnConnectionClosedHandler onConnectionClosedHandler) = 0;

    virtual bool start() = 0;
    virtual void stop() = 0;

    using OnSendCommandHandler = void (CpuComDaemon::*)(SessionID,
                                                        common::CpuCommand,
                                                        std::vector<uint8_t>);
    virtual void setSendCommandMessageHandler(OnSendCommandHandler handler,
                                              CpuComDaemon* daemon) = 0;

    using OnSubscribeHandler = void (CpuComDaemon::*)(SessionID, common::CpuCommand);
    virtual void setSubscribeMessageHandler(OnSubscribeHandler handler, CpuComDaemon* daemon) = 0;

    using OnUnsubscribeHandler = void (CpuComDaemon::*)(SessionID, common::CpuCommand);
    virtual void setUnsubscribeMessageHandler(OnUnsubscribeHandler handler,
                                              CpuComDaemon* daemon) = 0;

    using OnRequestHandler = void (CpuComDaemon::*)(SessionID,
                                                    common::UUID,
                                                    common::CpuCommand,
                                                    std::vector<uint8_t>,
                                                    common::CpuCommand);
    virtual void setRequestMessageHandler(OnRequestHandler handler, CpuComDaemon* daemon) = 0;

    using OnCancelRequestHandler = void (CpuComDaemon::*)(SessionID, common::UUID);
    virtual void setCancelRequestMessageHandler(OnCancelRequestHandler handler,
                                                CpuComDaemon* daemon) = 0;

    using OnSendCommandWithDeliveryStatusHandler = void (CpuComDaemon::*)(SessionID,
                                                                          common::UUID,
                                                                          common::CpuCommand,
                                                                          std::vector<uint8_t>);
    virtual void setSendCommandWithDeliveryStatusMessageHandler(
        OnSendCommandWithDeliveryStatusHandler handler,
        CpuComDaemon* daemon) = 0;

    virtual void sendNotificationMessage(SessionID sessionId,
                                         common::CpuCommand command,
                                         std::vector<uint8_t>& data) = 0;

    virtual void sendRequestResponseMessage(SessionID sessionId,
                                            common::UUID uuid,
                                            std::vector<uint8_t>& data) = 0;

    virtual void sendSendCommandResultMessage(SessionID sessionId,
                                              common::CpuCommand command,
                                              common::Error error) = 0;

    virtual void sendDeliveryStatusMessage(SessionID sessionId, common::UUID uuid, bool result) = 0;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IMESSAGESERVER_H
