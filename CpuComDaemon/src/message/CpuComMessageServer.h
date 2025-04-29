/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOM_MESSAGESERVER_H
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOM_MESSAGESERVER_H

#include "IMessageServer.h"

#include <Executors.h>

#include <memory>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class CpuComMessageServer final : public IMessageServer {  // LCOV_EXCL_LINE
    // exclude all possible destructors, that cannot be covered by unit-test
public:
    CpuComMessageServer(std::string socketName,
                        const std::unordered_set<std::string>& securityContextExceptions,
                        std::unique_ptr<common::IExecutor> incomingExecutor,
                        std::unique_ptr<common::IExecutor> outcomingExecutor,
                        OnStartedHandler onStartedHandler,
                        OnStoppedHandler onStoppedHandler);

    CpuComMessageServer(const CpuComMessageServer&) = delete;
    CpuComMessageServer(const CpuComMessageServer&&) = delete;
    CpuComMessageServer& operator=(const CpuComMessageServer&) = delete;
    CpuComMessageServer& operator=(const CpuComMessageServer&&) = delete;

    bool initialize(OnNewConnectionHandler onNewConnectionHandler,
                    OnConnectionClosedHandler onConnectionClosedHandler) override;

    bool start() override;
    void stop() override;

    void setSendCommandMessageHandler(OnSendCommandHandler handler, CpuComDaemon* daemon) override;

    void setSubscribeMessageHandler(OnSubscribeHandler handler, CpuComDaemon* daemon) override;

    void setUnsubscribeMessageHandler(OnUnsubscribeHandler handler, CpuComDaemon* daemon) override;

    void setRequestMessageHandler(OnRequestHandler handler, CpuComDaemon* daemon) override;

    void setCancelRequestMessageHandler(OnCancelRequestHandler handler,
                                        CpuComDaemon* daemon) override;

    void setSendCommandWithDeliveryStatusMessageHandler(
        OnSendCommandWithDeliveryStatusHandler handler,
        CpuComDaemon* daemon) override;

    void sendNotificationMessage(SessionID sessionId,
                                 common::CpuCommand command,
                                 std::vector<uint8_t>& data) override;

    void sendRequestResponseMessage(SessionID sessionId,
                                    common::UUID uuid,
                                    std::vector<uint8_t>& data) override;

    void sendSendCommandResultMessage(SessionID sessionId,
                                      common::CpuCommand command,
                                      common::Error error) override;

    void sendDeliveryStatusMessage(SessionID sessionId, common::UUID uuid, bool result) override;

private:
    std::unique_ptr<IMessageServer::MessageServer> mMessageServer;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOM_MESSAGESERVER_H
