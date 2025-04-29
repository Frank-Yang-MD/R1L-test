/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCKIMESSAGESERVER_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCKIMESSAGESERVER_H_

#include "IMessageServer.h"

#include <gmock/gmock.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class MockIMessageServer : public IMessageServer {
public:
    MOCK_METHOD0(start, bool());
    MOCK_METHOD0(stop, void());

    MOCK_METHOD2(initialize, bool(OnNewConnectionHandler, OnConnectionClosedHandler));

    MOCK_METHOD2(setSendCommandMessageHandler, void(OnSendCommandHandler, CpuComDaemon*));
    MOCK_METHOD2(setSubscribeMessageHandler, void(OnSubscribeHandler, CpuComDaemon*));
    MOCK_METHOD2(setUnsubscribeMessageHandler, void(OnUnsubscribeHandler, CpuComDaemon*));
    MOCK_METHOD2(setRequestMessageHandler, void(OnRequestHandler, CpuComDaemon*));
    MOCK_METHOD2(setCancelRequestMessageHandler, void(OnCancelRequestHandler, CpuComDaemon*));
    MOCK_METHOD2(setSendCommandWithDeliveryStatusMessageHandler,
                 void(OnSendCommandWithDeliveryStatusHandler, CpuComDaemon*));
    MOCK_METHOD3(sendNotificationMessage,
                 void(SessionID, common::CpuCommand, std::vector<uint8_t>&));
    MOCK_METHOD3(sendRequestResponseMessage, void(SessionID, common::UUID, std::vector<uint8_t>&));
    MOCK_METHOD3(sendSendCommandResultMessage, void(SessionID, common::CpuCommand, common::Error));
    MOCK_METHOD3(sendDeliveryStatusMessage, void(SessionID, common::UUID, bool));
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCKIMESSAGESERVER_H_
