/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_MOCKIMESSENGER_H_
#define COM_MITSUBISHIELECTRIC_AHU_MOCKIMESSENGER_H_

#include "message/IMessenger.h"

#include <gmock/gmock.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class MockIMessenger : public IMessenger {
public:
    MOCK_METHOD2(initialize, bool(OnConnectionClosedHandler, OnConnectionResumedHandler));
    MOCK_METHOD0(connect, bool());
    MOCK_METHOD0(disconnect, void());

    MOCK_METHOD2(setSendCommandResultMessageHandler, void(OnSendCommandResultHandler, v2::CpuCom*));
    MOCK_METHOD2(setNotificationMessageHandler, void(OnNotificationHandler, v2::CpuCom*));
    MOCK_METHOD2(setRequestResponseMessageHandler, void(OnRequestResponseHandler, v2::CpuCom*));
    MOCK_METHOD2(setDeliveryStatusMessageHandler, void(OnDeliveryStatusHandler, v2::CpuCom*));

    MOCK_METHOD1(sendCancelRequestMessage, void(common::UUID));
    MOCK_METHOD1(sendSubscribeMessage, void(common::CpuCommand));
    MOCK_METHOD1(sendUnsubscribeMessage, void(common::CpuCommand));
    MOCK_METHOD2(sendSendCommandMessage, void(common::CpuCommand, std::vector<uint8_t>));
    MOCK_METHOD3(sendSendCommandWithDeliveryStatusMessage,
                 void(common::UUID, common::CpuCommand, std::vector<uint8_t>));
    MOCK_METHOD4(sendRequestMessage,
                 void(common::UUID, common::CpuCommand, std::vector<uint8_t>, common::CpuCommand));
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_MOCKIMESSENGER_H_
