/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_PIMPL_CPUCOMIMPL_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_PIMPL_CPUCOMIMPL_H_

#include <unistd.h>

#include <functional>
#include <future>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "CpuCom.h"
#include "CpuComMessage.h"
#include "CpuCommand.h"
#include "Pack.h"
#include "Socket.h"
#include "ThreadPolicy.h"
#include "common/IoDevice.h"

#include "libCpuCom.h"

#define LOG_NDEBUG 1
#include "Log.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {

namespace cpucom {
namespace impl {

using common::MLOGD;
using common::MLOGW;

using common::CpuCommand;

using common::pack;
using common::unpack;

template <typename SendThreadPolicy, typename ReceiveThreadPolicy>
class CpuComImpl
    : private SendThreadPolicy
    , private ReceiveThreadPolicy {
public:
    explicit CpuComImpl(std::unique_ptr<common::Socket> socket, std::shared_ptr<IIoDevice> ioDevice)
        : SendThreadPolicy()
        , ReceiveThreadPolicy()
        , m_socket(std::move(socket))
        , m_ioDevice(std::move(ioDevice))
        , m_thread(nullptr)
        , m_errorCallback(nullptr)
        , m_stopfd(-1)
        , m_parser(new (std::nothrow) CpuComMessageParser())
    {
        m_socket->connect(kCpuComDaemonSocketName);
        if (m_socket->valid()) {
            MLOGD(common::FunctionID::cpuc_lib, LogID::ConnectedToDaemon,
                  m_socket->getFileDescriptor());
            m_stopfd = m_ioDevice->eventfd(0, 0);
            if (m_stopfd != -1) {
                startNotificationsThread();
            }
        }
        else {
            MLOGW(common::FunctionID::cpuc_lib, LogID::CouldNotConnectToDaemon,
                  kCpuComDaemonSocketName);
        }
    }

    ~CpuComImpl()
    {
        stopNotificationsThread();
        close(m_stopfd);
    }

public:
    void send(const common::CpuCommand& command, const std::vector<uint8_t>& data)
    {
        auto socket = m_socket;
        auto l = [socket](const CpuCommand& command, const std::vector<uint8_t>& data) {
            bool result = false;
            if (socket->valid()) {
                CpuComMessage m(CpuComId::SendCommand, pack(command, data));
                std::vector<uint8_t> message = m.serialize();
                result = static_cast<std::size_t>(socket->write(message)) == message.size();
            }
            else {
                result = false;
            }
            return result;
        };
        SendThreadPolicy::call(l, command, data);
    }

    void subscribe(const common::CpuCommand& command, ICpuCommandListener* listener)
    {
        if (m_listeners.find(command) == m_listeners.end()) {
            // we are not subscribed to this command,
            // send subscribe request to CpuComDaemon
            std::list<ICpuCommandListener*> listeners = {listener};
            {
                std::unique_lock<std::mutex> lock(m_listenersMutex);
                m_listeners.insert(std::make_pair(command, listeners));
            }
            auto socket = m_socket;
            auto l = [socket](const CpuCommand& command) {
                bool result = false;
                if (socket->valid()) {
                    CpuComMessage m(CpuComId::Subscribe, pack(command, std::vector<uint8_t>()));
                    std::vector<uint8_t> message = m.serialize();
                    result = static_cast<std::size_t>(socket->write(message)) == message.size();
                }
                else {
                    result = false;
                }
                return result;
            };
            SendThreadPolicy::call(l, command);
        }
        else {
            // we are already subscribed, just add another listener to the list
            std::unique_lock<std::mutex> lock(m_listenersMutex);
            m_listeners.at(command).push_back(listener);
        }
    }

    void subscribe(const std::list<common::CpuCommand>& commands, ICpuCommandListener* listener)
    {
        for (auto i = commands.begin(); i != commands.end(); ++i) {
            subscribe(*i, listener);
        }
    }

    void unsubscribe(const common::CpuCommand& command, ICpuCommandListener* listener)
    {
        bool unsubscribe = false;
        {
            std::unique_lock<std::mutex> lock(m_listenersMutex);
            if (m_listeners.find(command) != m_listeners.end()) {
                std::list<ICpuCommandListener*>& listeners = m_listeners.at(command);
                auto listenerToUnsubscribe =
                    std::find(listeners.begin(), listeners.end(), listener);
                if (listenerToUnsubscribe != listeners.end()) {
                    listeners.erase(listenerToUnsubscribe);
                    if (listeners.empty()) {
                        m_listeners.erase(command);
                        unsubscribe = true;
                    }
                }
            }
        }

        if (unsubscribe) {
            auto socket = m_socket;
            auto l = [socket](const CpuCommand& command) {
                bool result = false;
                if (socket->valid()) {
                    CpuComMessage m(CpuComId::Unsubscribe, pack(command, std::vector<uint8_t>()));
                    std::vector<uint8_t> message = m.serialize();
                    result = static_cast<std::size_t>(socket->write(message)) == message.size();
                }
                else {
                    result = false;
                }
                return result;
            };
            SendThreadPolicy::call(l, command);
        }
    }

    void unsubscribe(const std::list<common::CpuCommand>& commands, ICpuCommandListener* listener)
    {
        for (auto i = commands.begin(); i != commands.end(); ++i) {
            unsubscribe(*i, listener);
        }
    }

    void setErrorCallback(ICpuCommandErrorListener* callback) { m_errorCallback = callback; }

    void onReadyReadFromDaemon()
    {
        std::vector<uint8_t> data = m_socket->readAll();
        std::vector<CpuComMessage> messages = m_parser->parse(data);
        for (auto i = messages.begin(); i != messages.end(); ++i) {
            switch (i->type()) {
            case CpuComId::SendCommandResult:
                onSendCommandResult(i->data());
                break;
            case CpuComId::Notification:
                onNotification(i->data());
                break;
            default:
                break;
            }
        }
    }
    void onNotification(const std::vector<uint8_t>& data)
    {
        CpuCommand command;
        std::vector<uint8_t> commandData;
        if (unpack(data, command, commandData) == data.cend()) {
            std::list<ICpuCommandListener*> listeners;
            {
                std::unique_lock<std::mutex> lock(m_listenersMutex);
                if (m_listeners.find(command) != m_listeners.end()) {
                    listeners = m_listeners.at(command);
                }
            }
            for (auto i = listeners.begin(); i != listeners.end(); ++i) {
                ReceiveThreadPolicy::call(
                    std::bind(&ICpuCommandListener::onReceiveCommand, *i, command, commandData));
            }
        }
        else {
            MLOGW(common::FunctionID::cpuc_lib, LogID::WrongNotificationMessageSize);
        }
    }

    void onSendCommandResult(const std::vector<uint8_t>& data)
    {
        if (m_errorCallback) {
            CpuCommand command;
            int commandData;
            if (unpack(data, command, commandData) == data.cend()) {
                ReceiveThreadPolicy::call(std::bind(&ICpuCommandErrorListener::onError,
                                                    m_errorCallback, command, commandData));
            }
            else {
                MLOGW(common::FunctionID::cpuc_lib, LogID::WrongCommandResultMessageSize);
            }
        }
    }

    void startNotificationsThread()
    {
        m_thread.reset(new (std::nothrow)
                           std::thread(std::bind(&CpuComImpl::notificationsThreadFunction, this)));
    }

    void stopNotificationsThread()
    {
        if (m_stopfd != -1) {
            uint64_t stopFlag = 1;
            int bytes_written = ::write(m_stopfd, &stopFlag, sizeof(uint64_t));
            (void)bytes_written;
        }
        if (m_thread && m_thread->joinable()) {
            m_thread->join();
        }
    }

    void notificationsThreadFunction()
    {
        m_ioDevice->pollInit(m_socket->getFileDescriptor(), m_stopfd);

        while (true) {
            int action = m_ioDevice->poll();
            if (action == -1) {
                continue;
            }

            if (m_ioDevice->isSocketFdReceivePollInEvent()) {
                onReadyReadFromDaemon();
            }
            else if (m_ioDevice->isSocketFdReceivePollHupEvent()) {
                // POOL(2):
                // this event merely indicates that the peer  closed  its end of the channel.
                MLOGE(common::FunctionID::cpuc_lib, LogID::NotificationsThreadFunctionPoolHup);
                break;
            }

            if (m_ioDevice->isStopFdReceivePollInEvent()) {
                // stop request
                MLOGI(common::FunctionID::cpuc_lib, LogID::NotificationsThreadFunctionStopRequest);
                break;
            }
            else if (m_ioDevice->isStopFdReceivePollHupEvent()) {
                // POOL(2):
                // this event merely indicates that the peer  closed  its end of the channel.
                MLOGE(common::FunctionID::cpuc_lib,
                      LogID::NotificationsThreadFunctionPoolHupStopFd);
                break;
            }
        }
    }

private:
    std::shared_ptr<common::Socket> m_socket;
    std::shared_ptr<IIoDevice> m_ioDevice;
    std::unique_ptr<std::thread> m_thread;
    std::map<CpuCommand, std::list<ICpuCommandListener*>> m_listeners;
    std::mutex m_listenersMutex;
    ICpuCommandErrorListener* m_errorCallback;
    int m_stopfd;
    std::unique_ptr<CpuComMessageParser> m_parser;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_PIMPL_CPUCOMIMPL_H_
