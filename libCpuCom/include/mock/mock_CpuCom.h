/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCK_CPUCOM_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCK_CPUCOM_H_

#include "CpuCom.h"
#include <gmock/gmock.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {

class mock_ICpuCom : public ICpuCom {
public:
    MOCK_METHOD2(send, void(const common::CpuCommand& command, const std::vector<uint8_t>& data));
    MOCK_METHOD2(subscribe, void(const common::CpuCommand& command, ICpuCommandListener* listener));
    MOCK_METHOD2(subscribe,
                 void(const std::list<common::CpuCommand>& commands,
                      ICpuCommandListener* listener));
    MOCK_METHOD2(unsubscribe,
                 void(const common::CpuCommand& command, ICpuCommandListener* listener));
    MOCK_METHOD2(unsubscribe,
                 void(const std::list<common::CpuCommand>& commands,
                      ICpuCommandListener* listener));
    MOCK_METHOD1(setErrorCallback, void(ICpuCommandErrorListener* callback));
};

namespace v2 {

// Example usage of mock_ICpuComResponse
// cpucom::v2::mock_ICpuCom *p_mockICpuCom = new NiceMock<cpucom::v2::mock_ICpuCom>();
// cpucom::v2::mock_ICpuComResponse *p_mockICpuComResponse = new
// NiceMock<cpucom::v2::mock_ICpuComResponse>();

// ON_CALL(*p_mockICpuCom, request(_, _,
// _)).WillByDefault(Return(ByMove(std::unique_ptr<mock_ICpuComResponse>(p_mockICpuComResponse))));
// ON_CALL(*p_mockICpuComResponse, data()).WillByDefault(Return(std::vector<uint8_t>{0x01, 0x02,
// 0x03})); EXPECT_CALL(*p_mockICpuComResponse, wait()).Times(1);

class mock_ICpuComResponse : public ICpuComResponse {
public:
    MOCK_METHOD0(data, std::vector<uint8_t>());
    MOCK_METHOD0(wait, void());
    MOCK_METHOD1(wait_for, std::future_status(const std::chrono::milliseconds& timeout_duration));
};

class mock_ICpuCom : public ICpuCom {
public:
    MOCK_METHOD2(initialize,
                 bool(OnSendCommandError errorCallback, OnConnectionClosed onConnectionClosed));
    MOCK_METHOD0(connect, bool());
    MOCK_METHOD0(disconnect, void());
    MOCK_METHOD2(send, void(common::CpuCommand command, std::vector<uint8_t> data));
    MOCK_METHOD3(send,
                 void(common::CpuCommand command,
                      std::vector<uint8_t> data,
                      DeliveryStatusCallback deliveryStatusCallback));
    MOCK_METHOD3(request,
                 std::unique_ptr<ICpuComResponse>(common::CpuCommand requestCommand,
                                                  std::vector<uint8_t> requestData,
                                                  common::CpuCommand responseCommand));
    MOCK_METHOD2(subscribe, void(common::CpuCommand command, OnCommand callback));
    MOCK_METHOD2(subscribe, void(std::list<common::CpuCommand> commands, OnCommand callback));
    MOCK_METHOD1(unsubscribe, void(common::CpuCommand command));
    MOCK_METHOD1(unsubscribe, void(std::list<common::CpuCommand> commands));
};
}  // namespace v2

}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCK_CPUCOM_H_
