/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_IMESSAGER_H_
#define COM_MITSUBISHIELECTRIC_AHU_IMESSAGER_H_

#include "CpuComMessage.h"
#include "CpuCommand.h"
#include "UUID.h"
#include "messenger/Messenger.h"

#include <vector>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace v2 {
class CpuCom;
}
namespace impl {

class IMessenger {
public:
    using Messenger = common::Messenger<CpuComId>;
    using OnConnectionClosedHandler = Messenger::OnConnectionClosedHandler;
    using OnConnectionResumedHandler = Messenger::OnConnectionResumedHandler;

    // LCOV_EXCL_START
    virtual ~IMessenger() = default;
    // LCOV_EXCL_STOP

    virtual bool initialize(OnConnectionClosedHandler onConnectionClosed,
                            OnConnectionResumedHandler onConnectionResumed) = 0;

    virtual bool connect() = 0;
    virtual void disconnect() = 0;

    using OnSendCommandResultHandler = void (v2::CpuCom::*)(common::CpuCommand, int);
    virtual void setSendCommandResultMessageHandler(OnSendCommandResultHandler handler,
                                                    v2::CpuCom* cpuCom) = 0;

    using OnNotificationHandler = void (v2::CpuCom::*)(common::CpuCommand, std::vector<uint8_t>);
    virtual void setNotificationMessageHandler(OnNotificationHandler handler,
                                               v2::CpuCom* cpuCom) = 0;

    using OnRequestResponseHandler = void (v2::CpuCom::*)(common::UUID, std::vector<uint8_t>);
    virtual void setRequestResponseMessageHandler(OnRequestResponseHandler handler,
                                                  v2::CpuCom* cpuCom) = 0;

    using OnDeliveryStatusHandler = void (v2::CpuCom::*)(common::UUID, bool);
    virtual void setDeliveryStatusMessageHandler(OnDeliveryStatusHandler handler,
                                                 v2::CpuCom* cpuCom) = 0;

    virtual void sendCancelRequestMessage(common::UUID uuid) = 0;

    virtual void sendSubscribeMessage(common::CpuCommand command) = 0;

    virtual void sendUnsubscribeMessage(common::CpuCommand command) = 0;

    virtual void sendSendCommandMessage(common::CpuCommand command, std::vector<uint8_t> data) = 0;

    virtual void sendSendCommandWithDeliveryStatusMessage(common::UUID uuid,
                                                          common::CpuCommand command,
                                                          std::vector<uint8_t> data) = 0;

    virtual void sendRequestMessage(common::UUID uuid,
                                    common::CpuCommand requestCommand,
                                    std::vector<uint8_t> data,
                                    common::CpuCommand responseCommand) = 0;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_IMESSAGER_H_
