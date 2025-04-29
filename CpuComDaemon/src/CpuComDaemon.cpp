/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "CpuComDaemon.h"
#include "CPU.h"
#include "CpuComDaemonLog.h"

#include <iomanip>
#include <mutex>
#include <sstream>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {

namespace {
// LCOV_EXCL_START
// This is excluded from a unit test coverage report because this inner function
// that cannot be covered by unit-test
std::string to_string(const common::CpuCommand& command,
                      const std::vector<uint8_t>& data,
                      size_t maxDataSizeToPrint)
{
    std::stringstream ss;
    ss << "cmd=";
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(command.first);
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(command.second);
    ss << ", len=";
    ss << std::dec << std::setw(3) << std::setfill('0') << static_cast<int>(data.size());
    if (!data.empty()) {
        ss << ", dat=";
        size_t size = std::min(maxDataSizeToPrint, data.size());
        for (size_t i = 0; i < size; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
        }
    }
    return ss.str();
}
// LCOV_EXCL_STOP

const char* const kLogTagReceive = "V->M";
const char* const kLogTagSend = "M->V";
const size_t kMaxDataSizeToPrint = 253;  // print max to regular frame length
}  // namespace

using daemon::LogID;

using common::MLOGD;
using common::MLOGD_SERIAL;

using common::CpuCommand;
using namespace std::placeholders;

CpuComDaemon::CpuComDaemon(std::unique_ptr<impl::IMessageServer> messageServer,
                           std::unique_ptr<impl::ICPU> vcpu,
                           std::unique_ptr<common::IPeriodicTaskExecutor> periodicExecutor,
                           std::unique_ptr<IMutexWrapper> subscribersMutexWrapper,
                           std::unique_ptr<IMutexWrapper> requestsMutexWrapper)
    : m_messageServer(std::move(messageServer))
    , m_vcpu(std::move(vcpu))
    , m_periodicExecutor(std::move(periodicExecutor))
    , m_subscribersMutexWrapper(std::move(subscribersMutexWrapper))
    , m_requestsMutexWrapper(std::move(requestsMutexWrapper))
{
}

CpuComDaemon::~CpuComDaemon()
{
    m_running = false;
    m_periodicExecutor->stop();
    m_messageServer->stop();
}

bool CpuComDaemon::start()
{
    if (m_vcpu->initialize()) {
        m_running = true;
        m_periodicExecutor->submit(std::bind(&CpuComDaemon::vcpuThreadFunction, this),
                                   std::bind(&CpuComDaemon::isRunning, this));
    }

    m_messageServer->initialize(std::bind(&CpuComDaemon::onClientConnected, this, _1),
                                std::bind(&CpuComDaemon::onClientDisconnected, this, _1));

    m_messageServer->setSendCommandMessageHandler(&CpuComDaemon::onSendCommand, this);
    m_messageServer->setSubscribeMessageHandler(&CpuComDaemon::onSubscribe, this);
    m_messageServer->setUnsubscribeMessageHandler(&CpuComDaemon::onUnsubscribe, this);
    m_messageServer->setRequestMessageHandler(&CpuComDaemon::onRequest, this);
    m_messageServer->setCancelRequestMessageHandler(&CpuComDaemon::onCancelRequest, this);
    m_messageServer->setSendCommandWithDeliveryStatusMessageHandler(
        &CpuComDaemon::onSendCommandWithDeliveryStatus, this);

    return m_messageServer->start();
}

bool CpuComDaemon::isRunning() const { return m_running; }

void CpuComDaemon::vcpuThreadFunction()
{
    std::pair<common::CpuCommand, std::vector<uint8_t>> result;
    bool received = m_vcpu->read(result);
    if (received) {
        onReceiveCommand(std::move(result.first), std::move(result.second));
    }
}

void CpuComDaemon::onReceiveCommand(common::CpuCommand command, std::vector<uint8_t> data)
{
    MLOGD_SERIAL(kLogTagReceive, to_string(command, data, kMaxDataSizeToPrint).c_str());
    typename std::remove_reference<decltype(m_subscribers[command])>::type subscribers;
    {
        m_subscribersMutexWrapper->lock(m_subscribersMutex);
        subscribers = m_subscribers[command];
        m_subscribersMutexWrapper->unlock(m_subscribersMutex);
    }

    std::for_each(subscribers.begin(), subscribers.end(),
                  [this, &command, &data](const auto& sessionID) {
                      m_messageServer->sendNotificationMessage(sessionID, command, data);
                  });

    bool requestIsFound = false;
    typename decltype(m_requests)::value_type request = {};
    {
        auto findByCommand = [command](const auto& value) { return std::get<1>(value) == command; };

        m_requestsMutexWrapper->lock(m_requestsMutex);
        auto i = std::find_if(m_requests.begin(), m_requests.end(), findByCommand);
        requestIsFound = (i != m_requests.end());
        if (requestIsFound) {
            request = std::move(*i);
            m_requests.erase(i);
        }
        m_requestsMutexWrapper->unlock(m_requestsMutex);
    }
    if (requestIsFound) {
        MLOGI(common::FunctionID::cpuc_daemon, LogID::Response, std::get<0>(request).toString());
        m_messageServer->sendRequestResponseMessage(std::get<2>(request), std::get<0>(request),
                                                    data);
    }
}

void CpuComDaemon::onClientConnected(SessionID) {}

void CpuComDaemon::onClientDisconnected(SessionID sessionID)
{
    {
        m_subscribersMutexWrapper->lock(m_subscribersMutex);
        for (auto i = m_subscribers.begin(); i != m_subscribers.end(); ++i) {
            i->second.erase(sessionID);
        }
        m_subscribersMutexWrapper->unlock(m_subscribersMutex);
    }
    {
        auto findBySessionID = [sessionID](const auto& value) {
            return std::get<2>(value) == sessionID;
        };
        m_requestsMutexWrapper->lock(m_requestsMutex);
        m_requests.erase(std::remove_if(m_requests.begin(), m_requests.end(), findBySessionID),
                         m_requests.end());
        m_requestsMutexWrapper->unlock(m_requestsMutex);
    }
}

void CpuComDaemon::onSendCommand(SessionID sessionID,
                                 common::CpuCommand command,
                                 std::vector<uint8_t> data)
{
    MLOGD_SERIAL(kLogTagSend, to_string(command, data, kMaxDataSizeToPrint).c_str());
    if (m_vcpu->write(command, data) == false) {
        m_messageServer->sendSendCommandResultMessage(sessionID, command, common::ERR_BUSY);
    }
}

void CpuComDaemon::onSubscribe(SessionID sessionID, common::CpuCommand command)
{
    m_subscribersMutexWrapper->lock(m_subscribersMutex);
    auto& subsribers = m_subscribers[command];
    subsribers.insert(sessionID);
    MLOGI(common::FunctionID::cpuc_daemon, LogID::ClientSubscribed, 0x00, command.first,
          command.second);
    m_subscribersMutexWrapper->unlock(m_subscribersMutex);
}

void CpuComDaemon::onUnsubscribe(SessionID sessionID, common::CpuCommand command)
{
    m_subscribersMutexWrapper->lock(m_subscribersMutex);
    auto& subscribers = m_subscribers[command];
    subscribers.erase(sessionID);
    MLOGI(common::FunctionID::cpuc_daemon, LogID::ClientUnsubscribed, 0x00, command.first,
          command.second);
    m_subscribersMutexWrapper->unlock(m_subscribersMutex);
}

void CpuComDaemon::onRequest(SessionID sessionID,
                             common::UUID requestID,
                             common::CpuCommand requestCommand,
                             std::vector<uint8_t> requestData,
                             common::CpuCommand responseCommand)
{
    MLOGI(common::FunctionID::cpuc_daemon, LogID::Request, requestID.toString());
    m_requestsMutexWrapper->lock(m_requestsMutex);
    m_requests.emplace_back(requestID, responseCommand, sessionID);
    m_requestsMutexWrapper->unlock(m_requestsMutex);
    MLOGD_SERIAL(kLogTagSend, to_string(requestCommand, requestData, kMaxDataSizeToPrint).c_str());
    m_vcpu->write(requestCommand, requestData);
}

void CpuComDaemon::onCancelRequest(SessionID, common::UUID requestID)
{
    auto findById = [requestID](const auto& value) { return std::get<0>(value) == requestID; };
    m_requestsMutexWrapper->lock(m_requestsMutex);
    auto i = std::find_if(m_requests.begin(), m_requests.end(), findById);
    if (i != m_requests.end()) {
        MLOGI(common::FunctionID::cpuc_daemon, LogID::CancelRequest, requestID.toString());
        m_requests.erase(i);
    }
    m_requestsMutexWrapper->unlock(m_requestsMutex);
}

void CpuComDaemon::onSendCommandWithDeliveryStatus(SessionID sessionID,
                                                   common::UUID requestID,
                                                   common::CpuCommand command,
                                                   std::vector<uint8_t> data)
{
    MLOGD_SERIAL(kLogTagSend, to_string(command, data, kMaxDataSizeToPrint).c_str());
    bool result = m_vcpu->write(command, data);
    m_messageServer->sendDeliveryStatusMessage(sessionID, requestID, result);
}

}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
