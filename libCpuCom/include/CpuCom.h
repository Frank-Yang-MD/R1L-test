/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOM_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOM_H_

#include <functional>
#include <future>
#include <list>
#include <memory>
#include <vector>

#include "CpuCommand.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {

class ICpuCommandListener {
public:
    virtual ~ICpuCommandListener() = default;
    virtual void onReceiveCommand(const common::CpuCommand& command,
                                  const std::vector<uint8_t>& data) = 0;
};

class ICpuCommandErrorListener {
public:
    virtual ~ICpuCommandErrorListener() = default;
    virtual void onError(const common::CpuCommand& command, int errorCode) = 0;
};

class ICpuCom {
public:
    explicit ICpuCom() = default;
    virtual ~ICpuCom() = default;
    ICpuCom(const ICpuCom&) = delete;
    ICpuCom& operator=(const ICpuCom&) = delete;
    ICpuCom(ICpuCom&&) = delete;
    ICpuCom& operator=(ICpuCom&&) = delete;

    static std::unique_ptr<ICpuCom> create();

    virtual void send(const common::CpuCommand& command, const std::vector<uint8_t>& data) = 0;
    virtual void subscribe(const common::CpuCommand& command, ICpuCommandListener* listener) = 0;
    virtual void subscribe(const std::list<common::CpuCommand>& commands,
                           ICpuCommandListener* listener) = 0;
    virtual void unsubscribe(const common::CpuCommand& command, ICpuCommandListener* listener) = 0;
    virtual void unsubscribe(const std::list<common::CpuCommand>& commands,
                             ICpuCommandListener* listener) = 0;
    virtual void setErrorCallback(ICpuCommandErrorListener* callback) = 0;
};

namespace v2 {

class ICpuComResponse {
public:
    virtual ~ICpuComResponse() = default;

public:
    virtual std::vector<uint8_t> data() = 0;
    virtual void wait() = 0;
    virtual std::future_status wait_for(const std::chrono::milliseconds& timeout_duration) = 0;
};

class ICpuCom {
public:
    using OnCommand = std::function<void(common::CpuCommand, std::vector<uint8_t>)>;
    using OnSendCommandError = std::function<void(common::CpuCommand, int errorCode)>;
    using OnConnectionClosed = std::function<void()>;
    using DeliveryStatusCallback = std::function<void(bool)>;

public:
    static std::unique_ptr<ICpuCom> create();
    virtual ~ICpuCom() = default;

    virtual bool initialize(OnSendCommandError errorCallback,
                            OnConnectionClosed onConnectionClosed) = 0;

    virtual bool connect() = 0;
    virtual void disconnect() = 0;

    virtual void send(common::CpuCommand command, std::vector<uint8_t> data) = 0;
    virtual void send(common::CpuCommand command,
                      std::vector<uint8_t> data,
                      DeliveryStatusCallback deliveryStatusCallback) = 0;
    virtual std::unique_ptr<ICpuComResponse> request(common::CpuCommand requestCommand,
                                                     std::vector<uint8_t> requestData,
                                                     common::CpuCommand responseCommand) = 0;

    virtual void subscribe(common::CpuCommand command, OnCommand callback) = 0;
    virtual void subscribe(std::list<common::CpuCommand> commands, OnCommand callback) = 0;
    virtual void unsubscribe(common::CpuCommand command) = 0;
    virtual void unsubscribe(std::list<common::CpuCommand> commands) = 0;
};
}  // namespace v2

}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_CPUCOM_H_
