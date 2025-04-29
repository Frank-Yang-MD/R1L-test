/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOMMESSAGER_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOMMESSAGER_H_

#include "IMessenger.h"

#include <Executors.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class CpuComMessenger final : public IMessenger {  // LCOV_EXCL_LINE
    // exclude all possible destructors, that cannot be covered by unit-test
public:
    CpuComMessenger(std::string daemonSocketName,
                    std::unique_ptr<common::Socket> socket,
                    std::unique_ptr<common::IPausableExecutor> incomingExecutor,
                    std::unique_ptr<common::IExecutor> outcomingExecutor,
                    bool reconnect);

    CpuComMessenger(const CpuComMessenger&) = delete;
    CpuComMessenger(const CpuComMessenger&&) = delete;
    CpuComMessenger& operator=(const CpuComMessenger&) = delete;
    CpuComMessenger& operator=(const CpuComMessenger&&) = delete;

    bool initialize(OnConnectionClosedHandler onConnectionClosed,
                    OnConnectionResumedHandler onConnectionResumed) override;

    bool connect() override;
    void disconnect() override;

    void setSendCommandResultMessageHandler(OnSendCommandResultHandler handler,
                                            v2::CpuCom* cpuCom) override;

    void setNotificationMessageHandler(OnNotificationHandler handler, v2::CpuCom* cpuCom) override;

    void setRequestResponseMessageHandler(OnRequestResponseHandler handler,
                                          v2::CpuCom* cpuCom) override;

    void setDeliveryStatusMessageHandler(OnDeliveryStatusHandler handler,
                                         v2::CpuCom* cpuCom) override;

    void sendCancelRequestMessage(common::UUID uuid) override;

    void sendSubscribeMessage(common::CpuCommand command) override;

    void sendUnsubscribeMessage(common::CpuCommand command) override;

    void sendSendCommandMessage(common::CpuCommand command, std::vector<uint8_t> data) override;

    void sendSendCommandWithDeliveryStatusMessage(common::UUID uuid,
                                                  common::CpuCommand command,
                                                  std::vector<uint8_t> data) override;

    void sendRequestMessage(common::UUID uuid,
                            common::CpuCommand requestCommand,
                            std::vector<uint8_t> data,
                            common::CpuCommand responseCommand) override;

private:
    std::unique_ptr<IMessenger::Messenger> mMessenger;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOMMESSAGER_H_
