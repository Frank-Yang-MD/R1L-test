/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOMDAEMON_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOMDAEMON_H_

#include <atomic>
#include <memory>
#include <set>

#include <PeriodicTaskExecutor.h>

#include "IMessageServer.h"
#include "IMutexWrapper.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {
class ICPU;
}  // namespace impl

using SessionID = impl::IMessageServer::SessionID;

class CpuComDaemon {
public:
    explicit CpuComDaemon(std::unique_ptr<impl::IMessageServer> messageServer,
                          std::unique_ptr<impl::ICPU> vcpu,
                          std::unique_ptr<common::IPeriodicTaskExecutor> periodicExecutor,
                          std::unique_ptr<IMutexWrapper> subscribersMutexWrapper,
                          std::unique_ptr<IMutexWrapper> requestsMutexWrapper);
    ~CpuComDaemon();

    bool start();

    void onClientConnected(SessionID sessionID);
    void onClientDisconnected(SessionID sessionID);
    void onSendCommand(SessionID sessionID, common::CpuCommand command, std::vector<uint8_t> data);
    void onSubscribe(SessionID sessionID, common::CpuCommand command);
    void onUnsubscribe(SessionID sessionID, common::CpuCommand command);
    void onRequest(SessionID sessionID,
                   common::UUID requestID,
                   common::CpuCommand requestCommand,
                   std::vector<uint8_t> requestData,
                   common::CpuCommand responseCommand);
    void onCancelRequest(SessionID sessionID, common::UUID requestID);
    void onSendCommandWithDeliveryStatus(SessionID,
                                         common::UUID requestID,
                                         common::CpuCommand command,
                                         std::vector<uint8_t> data);

private:
    void vcpuThreadFunction();
    void onReceiveCommand(common::CpuCommand command, std::vector<uint8_t> data);

    bool isRunning() const;

    std::unique_ptr<impl::IMessageServer> m_messageServer;
    std::unique_ptr<impl::ICPU> m_vcpu;
    std::unique_ptr<common::IPeriodicTaskExecutor> m_periodicExecutor;
    std::unique_ptr<IMutexWrapper> m_subscribersMutexWrapper;
    std::unique_ptr<IMutexWrapper> m_requestsMutexWrapper;
    std::map<common::CpuCommand, std::set<SessionID>> m_subscribers;
    std::mutex m_subscribersMutex;
    std::vector<std::tuple<common::UUID, common::CpuCommand, SessionID>> m_requests;
    std::mutex m_requestsMutex;
    std::atomic_bool m_running;
};

}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOMDAEMON_H_
