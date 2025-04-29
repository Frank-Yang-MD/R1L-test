/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "CpuComImpl_v2.h"
#include "libCpuCom.h"

#include <poll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Log.h"
#include "Pack.h"
#include "Socket.h"
#include "message/CpuComMessenger.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace v2 {

using common::CpuCommand;
using common::MLOGD;
using common::MLOGV;
using common::MLOGW;
using common::pack;
using common::unpack;
using common::UUID;

using impl::CpuComId;

using namespace std::placeholders;

std::unique_ptr<ICpuCom> ICpuCom::create()
{
    std::unique_ptr<common::Socket> socket(new (std::nothrow) common::Socket());
    std::unique_ptr<impl::IMessenger> messenger = std::make_unique<impl::CpuComMessenger>(
        impl::kCpuComDaemonSocketName, std::move(socket),
        std::make_unique<common::PausableSingleThreadExecutor>(),
        std::make_unique<common::SingleThreadExecutor>(), true);
    return std::make_unique<CpuCom>(std::move(messenger));
}

CpuComResponse::CpuComResponse(std::future<std::vector<uint8_t>>&& f, std::function<void()> deleter)
    : m_future(new (std::nothrow) std::future<std::vector<uint8_t>>(std::move(f)))
    , m_deleter(deleter)
{
}

CpuComResponse::~CpuComResponse()
{
    m_future.reset();
    if (m_deleter) {
        m_deleter();
    }
}

std::vector<uint8_t> CpuComResponse::data() { return m_future->get(); }

void CpuComResponse::wait() { return m_future->wait(); }

std::future_status CpuComResponse::wait_for(const std::chrono::milliseconds& timeout_duration)
{
    return m_future->wait_for(timeout_duration);
}

CpuCom::CpuCom(std::unique_ptr<impl::IMessenger> messenger)
    : m_messenger(std::move(messenger))
{
    auto cleanup = [=](UUID id) {
        bool canceled = false;
        {
            std::lock_guard<std::mutex> lock(m_requestsMutex);
            std::size_t count = m_requests.erase(id);
            canceled = count > 0;
        }
        if (canceled) {
            MLOGD(common::FunctionID::cpuc_lib, LogID::CancelRequest, id.toString());
            m_messenger->sendCancelRequestMessage(std::move(id));
        }
    };
    m_cancelFunc = std::make_shared<std::function<void(UUID)>>(cleanup);
}

CpuCom::~CpuCom()
{
    // CpuCom can be terminated only when responses will be received
    // for all requests or unresponded requests will be canceled
    std::lock_guard<std::mutex> lock(m_requestsMutex);
    for (auto& request : m_requests) {
        std::vector<uint8_t> empty;
        request.second.set_value(empty);
    }
}

bool CpuCom::initialize(OnSendCommandError errorCallback, OnConnectionClosed onConnectionClosed)
{
    m_errorCallback = errorCallback;
    m_onConnectionClosed = onConnectionClosed;

    auto onConnectionClosedHandler = [this]() {
        MLOGD(common::FunctionID::cpuc_lib, LogID::NotifyClientConnectionClosed);
        if (m_onConnectionClosed) {
            m_onConnectionClosed();
        }
    };

    auto onConnectionResumedHandler = [this]() {
        MLOGD(common::FunctionID::cpuc_lib, LogID::ResubscribeAndResume);
        for (auto i = m_callbacks.begin(); i != m_callbacks.end(); ++i) {
            m_messenger->sendSubscribeMessage(i->first);
        }
    };

    bool initialized =
        m_messenger->initialize(onConnectionClosedHandler, onConnectionResumedHandler);

    m_messenger->setSendCommandResultMessageHandler(&CpuCom::onSendCommandResult, this);
    m_messenger->setNotificationMessageHandler(&CpuCom::onNotification, this);
    m_messenger->setRequestResponseMessageHandler(&CpuCom::onRequestResponse, this);
    m_messenger->setDeliveryStatusMessageHandler(&CpuCom::onDeliveryStatus, this);

    return initialized;
}

bool CpuCom::connect() { return m_messenger->connect(); }

void CpuCom::disconnect() { m_messenger->disconnect(); }

void CpuCom::send(CpuCommand command, std::vector<uint8_t> data)
{
    MLOGD(common::FunctionID::cpuc_lib, LogID::Send, command.first, command.second);
    m_messenger->sendSendCommandMessage(std::move(command), std::move(data));
}

void CpuCom::send(common::CpuCommand command,
                  std::vector<uint8_t> data,
                  DeliveryStatusCallback deliveryStatusCallback)
{
    UUID id;
    std::unique_lock<std::mutex> lock(m_deliveryStatusCallbacksMutex);
    m_deliveryStatusCallbacks.insert(std::make_pair(id, deliveryStatusCallback));
    lock.unlock();

    MLOGD(common::FunctionID::cpuc_lib, LogID::SendWithDeliveryConfirmation, command.first,
          command.second, id.toString());
    m_messenger->sendSendCommandWithDeliveryStatusMessage(std::move(id), std::move(command),
                                                          std::move(data));
}

std::unique_ptr<ICpuComResponse> CpuCom::request(CpuCommand requestCommand,
                                                 std::vector<uint8_t> requestData,
                                                 CpuCommand responseCommand)
{
    UUID id;
    std::weak_ptr<std::function<void(UUID)>> f = m_cancelFunc;
    auto cleanup = [=]() {
        auto x = f.lock();
        if (x) {
            x->operator()(id);
        }
    };

    std::promise<std::vector<uint8_t>> result;
    std::unique_ptr<ICpuComResponse> response =
        std::make_unique<CpuComResponse>(result.get_future(), cleanup);
    std::unique_lock<std::mutex> lock(m_requestsMutex);
    m_requests.insert(std::make_pair(id, std::move(result)));
    lock.unlock();

    MLOGD(common::FunctionID::cpuc_lib, LogID::Request, id.toString());
    m_messenger->sendRequestMessage(std::move(id), std::move(requestCommand),
                                    std::move(requestData), std::move(responseCommand));
    return response;
}

void CpuCom::subscribe(CpuCommand command, OnCommand callback)
{
    std::unique_lock<std::mutex> lock(m_callbacksMutex);
    bool result = m_callbacks.find(command) == m_callbacks.end();
    if (result) {
        m_callbacks.insert(std::make_pair(command, callback));
        lock.unlock();
        m_messenger->sendSubscribeMessage(std::move(command));
    }
}

void CpuCom::subscribe(std::list<common::CpuCommand> commands, OnCommand callback)
{
    for (auto i = commands.begin(); i != commands.end(); ++i) {
        subscribe(*i, callback);
    }
}

void CpuCom::unsubscribe(CpuCommand command)
{
    std::unique_lock<std::mutex> lock(m_callbacksMutex);
    m_callbacks.erase(command);
    lock.unlock();

    m_messenger->sendUnsubscribeMessage(std::move(command));
}

void CpuCom::unsubscribe(std::list<common::CpuCommand> commands)
{
    for (auto i = commands.begin(); i != commands.end(); ++i) {
        unsubscribe(*i);
    }
}

void CpuCom::onNotification(CpuCommand command, std::vector<uint8_t> data)
{
    MLOGD(common::FunctionID::cpuc_lib, LogID::Receive, command.first, command.second);
    OnCommand callback;
    {
        std::lock_guard<std::mutex> lock(m_callbacksMutex);
        auto c = m_callbacks.find(command);
        if (c != m_callbacks.end()) {
            callback = c->second;
        }
    }
    if (callback) {
        callback(std::move(command), std::move(data));
    }
}

void CpuCom::onSendCommandResult(CpuCommand command, int result)
{
    MLOGW(common::FunctionID::cpuc_lib, LogID::SendFailed, command.first, command.second);
    if (m_errorCallback) {
        m_errorCallback(std::move(command), result);
    }
}

void CpuCom::onRequestResponse(UUID id, std::vector<uint8_t> data)
{
    MLOGD(common::FunctionID::cpuc_lib, LogID::Response, id.toString());
    std::lock_guard<std::mutex> lock(m_requestsMutex);
    auto i = m_requests.find(id);
    bool found = (i != m_requests.end());
    if (found) {
        i->second.set_value(data);
        MLOGD(common::FunctionID::cpuc_lib, LogID::ResponseDelivered, id.toString());
    }
    else {
        MLOGD(common::FunctionID::cpuc_lib, LogID::ResponseDropped, id.toString());
    }
    m_requests.erase(id);
}

void CpuCom::onDeliveryStatus(UUID id, bool status)
{
    std::unique_lock<std::mutex> lock(m_deliveryStatusCallbacksMutex);
    DeliveryStatusCallback callback = m_deliveryStatusCallbacks[id];
    m_deliveryStatusCallbacks.erase(id);
    lock.unlock();

    if (callback) {
        MLOGD(common::FunctionID::cpuc_lib, LogID::DeliveryStatusProvided, id.toString());
        callback(status);
    }
    else {
        MLOGD(common::FunctionID::cpuc_lib, LogID::DeliveryStatusDropped, id.toString());
    }
}

}  // namespace v2
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
