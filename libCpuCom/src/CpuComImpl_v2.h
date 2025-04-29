/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOMIMPL_V2_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOMIMPL_V2_H_

#include "CpuCom.h"

#include <map>
#include <memory>
#include <mutex>

#include "message/IMessenger.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace common {
template <typename T>
class Messenger;
}
namespace cpucom {
namespace impl {
class SingleThreadExecutor;
}
namespace v2 {

class CpuComResponse final : public ICpuComResponse {
public:
    explicit CpuComResponse(std::future<std::vector<uint8_t>>&& f, std::function<void()> deleter);
    ~CpuComResponse();

    std::vector<uint8_t> data() override;

    void wait() override;

    std::future_status wait_for(const std::chrono::milliseconds& timeout_duration) override;

private:
    std::unique_ptr<std::future<std::vector<uint8_t>>> m_future;
    std::function<void()> m_deleter;
};

class CpuCom : public ICpuCom {
public:
    explicit CpuCom(std::unique_ptr<impl::IMessenger> messenger);
    virtual ~CpuCom();

    virtual bool initialize(OnSendCommandError errorCallback,
                            OnConnectionClosed onConnectionClosed);
    virtual bool connect() override;
    virtual void disconnect() override;

    virtual void send(common::CpuCommand command, std::vector<uint8_t> data) override;
    virtual void send(common::CpuCommand command,
                      std::vector<uint8_t> data,
                      DeliveryStatusCallback deliveryStatusCallback) override;
    virtual std::unique_ptr<ICpuComResponse> request(common::CpuCommand requestCommand,
                                                     std::vector<uint8_t> requestData,
                                                     common::CpuCommand responseCommand) override;
    virtual void subscribe(common::CpuCommand command, OnCommand callback) override;
    virtual void subscribe(std::list<common::CpuCommand> commands, OnCommand callback) override;
    virtual void unsubscribe(common::CpuCommand command) override;
    virtual void unsubscribe(std::list<common::CpuCommand> commands) override;

    void onNotification(common::CpuCommand command, std::vector<uint8_t> data);
    void onSendCommandResult(common::CpuCommand command, int result);
    void onRequestResponse(common::UUID id, std::vector<uint8_t> data);
    void onDeliveryStatus(common::UUID id, bool status);

private:
    OnSendCommandError m_errorCallback;
    OnConnectionClosed m_onConnectionClosed;

    std::map<common::CpuCommand, OnCommand> m_callbacks;
    std::mutex m_callbacksMutex;

    std::map<common::UUID, std::promise<std::vector<uint8_t>>> m_requests;
    std::mutex m_requestsMutex;
    std::shared_ptr<std::function<void(common::UUID)>> m_cancelFunc;

    std::map<common::UUID, DeliveryStatusCallback> m_deliveryStatusCallbacks;
    std::mutex m_deliveryStatusCallbacksMutex;

    std::unique_ptr<impl::IMessenger> m_messenger;
};

}  // namespace v2
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOMIMPL_V2_H_
