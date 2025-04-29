/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "libCpuCom.h"
#include "libMelcoCommon.h"

#include "CpuComImpl_v2.h"
#include "message/MockIMessenger.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SaveArg;

class MockUUIDMessenger : public MockIMessenger {
public:
    void sendSendCommandWithDeliveryStatusMessage(common::UUID uuid,
                                                  common::CpuCommand command,
                                                  std::vector<uint8_t> data) override
    {
        MockIMessenger::sendSendCommandWithDeliveryStatusMessage(uuid, command, data);
        m_sendUuid = uuid;
    }

    void sendRequestMessage(common::UUID uuid,
                            common::CpuCommand requestCommand,
                            std::vector<uint8_t> data,
                            common::CpuCommand responseCommand) override
    {
        MockIMessenger::sendRequestMessage(uuid, requestCommand, data, responseCommand);
        m_requestUuid = uuid;
    }

    const common::UUID& getSendUuid() const { return m_sendUuid; }

    const common::UUID& getRequestUuid() const { return m_requestUuid; }

private:
    common::UUID m_sendUuid;
    common::UUID m_requestUuid;
};

class libCpuComV2Test : public ::testing::Test {
protected:
    libCpuComV2Test()
        : m_testSendCommand(std::make_pair(0x01, 0x01))
        , m_testSendMessageData({0x01, 0x02, 0x03})
        , m_testSendCommandWithDeliveryStatusCommand1(std::make_pair(0x06, 0x06))
        , m_testSendCommandWithDeliveryStatusCommand2(std::make_pair(0x06, 0x07))
        , m_testSendCommandWithDeliveryStatusMessageData({0x04, 0x05, 0x06})
        , m_testRequestCommand1(std::make_pair(0x08, 0x08))
        , m_testRequestCommand2(std::make_pair(0x08, 0x09))
        , m_testRequestMessageData({0x07, 0x08, 0x09})
        , m_testResponseCommand(std::make_pair(0x09, 0x09))
        , m_testNotificationCommand(std::make_pair(0x05, 0x05))
        , m_testNotificationMessageData({0x03, 0x02, 0x01})
        , m_testSendResultCommand(std::make_pair(0x02, 0x02))
        , m_testSubscribeCommand1(std::make_pair(0x03, 0x03))
        , m_testSubscribeCommand2(std::make_pair(0x03, 0x04))
        , m_testUnsubscribeCommand1(std::make_pair(0x04, 0x04))
        , m_testUnsubscribeCommand2(std::make_pair(0x04, 0x05))
        , m_testSubscribeCommandsList({m_testSubscribeCommand1, m_testSubscribeCommand2})
        , m_testUnsubscribeCommandsList({m_testUnsubscribeCommand1, m_testUnsubscribeCommand2})
    {
        m_testDeliveryStatusCallback = [](bool) {};
        m_testSendCommandErrorCallback = [](common::CpuCommand, int) {};
        m_testConnectionClosedCallback = []() {};
        m_testCommandCallback = [](common::CpuCommand, std::vector<uint8_t>) {};
    }

    ~libCpuComV2Test() {}

    void SetUp()
    {
        common::InitializeCommonLogMessages();
        cpucom::InitializeLibCpuComLogMessages();
    }

    void TearDown()
    {
        cpucom::TerminateLibCpuComLogMessages();
        common::TerminateCommonLogMessages();
    }

    common::CpuCommand m_testSendCommand;
    std::vector<uint8_t> m_testSendMessageData;

    common::CpuCommand m_testSendCommandWithDeliveryStatusCommand1;
    common::CpuCommand m_testSendCommandWithDeliveryStatusCommand2;
    std::vector<uint8_t> m_testSendCommandWithDeliveryStatusMessageData;

    common::CpuCommand m_testRequestCommand1;
    common::CpuCommand m_testRequestCommand2;
    std::vector<uint8_t> m_testRequestMessageData;
    common::CpuCommand m_testResponseCommand;

    common::CpuCommand m_testNotificationCommand;
    std::vector<uint8_t> m_testNotificationMessageData;
    common::CpuCommand m_testSendResultCommand;

    common::CpuCommand m_testSubscribeCommand1;
    common::CpuCommand m_testSubscribeCommand2;

    common::CpuCommand m_testUnsubscribeCommand1;
    common::CpuCommand m_testUnsubscribeCommand2;

    std::list<common::CpuCommand> m_testSubscribeCommandsList;
    std::list<common::CpuCommand> m_testUnsubscribeCommandsList;

    v2::ICpuCom::DeliveryStatusCallback m_testDeliveryStatusCallback;
    v2::ICpuCom::OnSendCommandError m_testSendCommandErrorCallback;
    v2::ICpuCom::OnConnectionClosed m_testConnectionClosedCallback;
    v2::ICpuCom::OnCommand m_testCommandCallback;
};

TEST_F(libCpuComV2Test, createSingleInstanceTest)
{
    auto cpucom = v2::ICpuCom::create();

    cpucom->initialize(m_testSendCommandErrorCallback, m_testConnectionClosedCallback);
}

TEST_F(libCpuComV2Test, createCpuComResponseTest)
{
    using namespace std::chrono_literals;

    std::vector<uint8_t> data;
    std::promise<std::vector<uint8_t>> result;
    std::function<void()> deleter;

    std::unique_ptr<v2::ICpuComResponse> response(
        new v2::CpuComResponse(result.get_future(), deleter));

    result.set_value(data);

    response->wait();
    response->wait_for(5ms);

    EXPECT_EQ(response->data(), data);
}

TEST_F(libCpuComV2Test, subscribeAndInitializeTest)
{
    auto messenger = std::make_unique<NiceMock<MockIMessenger>>();
    NiceMock<MockIMessenger>* messengerRaw = messenger.get();

    std::function<void()> onConnectionClosedHandler;
    std::function<void()> onConnectionResumedHandler;
    v2::ICpuCom::OnConnectionClosed connectionClosedCallback;

    EXPECT_CALL(*messengerRaw, sendSubscribeMessage(m_testSubscribeCommand1)).Times(2);
    EXPECT_CALL(*messengerRaw, sendSubscribeMessage(m_testSubscribeCommand2)).Times(1);

    EXPECT_CALL(*messengerRaw, initialize(_, _))
        .Times(2)
        .WillRepeatedly(DoAll(SaveArg<0>(&onConnectionClosedHandler),
                              SaveArg<1>(&onConnectionResumedHandler), Return(true)));

    v2::CpuCom cpucom{std::move(messenger)};

    EXPECT_CALL(*messengerRaw, setSendCommandResultMessageHandler(_, &cpucom)).Times(2);
    EXPECT_CALL(*messengerRaw, setNotificationMessageHandler(_, &cpucom)).Times(2);
    EXPECT_CALL(*messengerRaw, setRequestResponseMessageHandler(_, &cpucom)).Times(2);
    EXPECT_CALL(*messengerRaw, setDeliveryStatusMessageHandler(_, &cpucom)).Times(2);

    cpucom.subscribe(m_testSubscribeCommand1, m_testCommandCallback);
    cpucom.initialize(m_testSendCommandErrorCallback, connectionClosedCallback);
    onConnectionClosedHandler();
    onConnectionResumedHandler();

    connectionClosedCallback = []() {};
    cpucom.subscribe(m_testSubscribeCommand2, m_testCommandCallback);
    cpucom.initialize(m_testSendCommandErrorCallback, connectionClosedCallback);
    onConnectionClosedHandler();
}

TEST_F(libCpuComV2Test, connectTest)
{
    auto messenger = std::make_unique<NiceMock<MockIMessenger>>();
    NiceMock<MockIMessenger>* messengerRaw = messenger.get();

    EXPECT_CALL(*messengerRaw, connect()).Times(1).WillOnce(Return(true));

    v2::CpuCom cpucom{std::move(messenger)};

    cpucom.connect();
}

TEST_F(libCpuComV2Test, disconnectTest)
{
    auto messenger = std::make_unique<NiceMock<MockIMessenger>>();
    NiceMock<MockIMessenger>* messengerRaw = messenger.get();

    EXPECT_CALL(*messengerRaw, disconnect()).Times(1);

    v2::CpuCom cpucom{std::move(messenger)};

    cpucom.disconnect();
}

TEST_F(libCpuComV2Test, sendSendCommandMessageTest)
{
    auto messenger = std::make_unique<NiceMock<MockIMessenger>>();
    NiceMock<MockIMessenger>* messengerRaw = messenger.get();

    EXPECT_CALL(*messengerRaw, sendSendCommandMessage(_, _)).Times(1);

    v2::CpuCom cpucom{std::move(messenger)};

    cpucom.send(m_testSendCommand, m_testSendMessageData);
}

TEST_F(libCpuComV2Test, sendSendCommandWithDeliveryStatusMessageTest)
{
    auto messenger = std::make_unique<NiceMock<MockIMessenger>>();
    NiceMock<MockIMessenger>* messengerRaw = messenger.get();

    v2::CpuCom cpucom{std::move(messenger)};

    EXPECT_CALL(*messengerRaw, sendSendCommandWithDeliveryStatusMessage(_, _, _)).Times(1);

    cpucom.send(m_testSendCommandWithDeliveryStatusCommand1,
                m_testSendCommandWithDeliveryStatusMessageData, m_testDeliveryStatusCallback);
}

TEST_F(libCpuComV2Test, requestTest)
{
    auto messenger = std::make_unique<NiceMock<MockIMessenger>>();
    NiceMock<MockIMessenger>* messengerRaw = messenger.get();

    EXPECT_CALL(*messengerRaw, sendRequestMessage(_, _, _, _)).Times(1);

    v2::CpuCom cpucom{std::move(messenger)};

    cpucom.request(m_testRequestCommand1, m_testRequestMessageData, m_testResponseCommand);
}

TEST_F(libCpuComV2Test, requestWithEarlyDestroyInstanceTest)
{
    auto messenger = std::make_unique<NiceMock<MockIMessenger>>();
    NiceMock<MockIMessenger>* messengerRaw = messenger.get();

    EXPECT_CALL(*messengerRaw, sendRequestMessage(_, _, _, _)).Times(1);

    std::unique_ptr<cpucom::v2::ICpuComResponse> response;

    {
        v2::CpuCom cpucom{std::move(messenger)};

        response =
            cpucom.request(m_testRequestCommand1, m_testRequestMessageData, m_testResponseCommand);
    }
}

TEST_F(libCpuComV2Test, subscribeTest)
{
    auto messenger = std::make_unique<NiceMock<MockIMessenger>>();
    NiceMock<MockIMessenger>* messengerRaw = messenger.get();

    EXPECT_CALL(*messengerRaw, sendSubscribeMessage(_)).Times(1);

    v2::CpuCom cpucom{std::move(messenger)};

    cpucom.subscribe(m_testSubscribeCommand1, m_testCommandCallback);
    cpucom.subscribe(m_testSubscribeCommand1, m_testCommandCallback);
}

TEST_F(libCpuComV2Test, subscribeListTest)
{
    auto messenger = std::make_unique<NiceMock<MockIMessenger>>();
    NiceMock<MockIMessenger>* messengerRaw = messenger.get();

    EXPECT_CALL(*messengerRaw, sendSubscribeMessage(_)).Times(2);

    v2::CpuCom cpucom{std::move(messenger)};

    cpucom.subscribe(m_testSubscribeCommandsList, m_testCommandCallback);
}

TEST_F(libCpuComV2Test, unsubscribeTest)
{
    auto messenger = std::make_unique<NiceMock<MockIMessenger>>();
    NiceMock<MockIMessenger>* messengerRaw = messenger.get();

    EXPECT_CALL(*messengerRaw, sendUnsubscribeMessage(_)).Times(1);

    v2::CpuCom cpucom{std::move(messenger)};

    cpucom.unsubscribe(m_testUnsubscribeCommand1);
}

TEST_F(libCpuComV2Test, unsubscribeListTest)
{
    auto messenger = std::make_unique<NiceMock<MockIMessenger>>();
    NiceMock<MockIMessenger>* messengerRaw = messenger.get();

    EXPECT_CALL(*messengerRaw, sendUnsubscribeMessage(_)).Times(2);

    v2::CpuCom cpucom{std::move(messenger)};

    cpucom.unsubscribe(m_testUnsubscribeCommandsList);
}

TEST_F(libCpuComV2Test, onNotificationTest)
{
    auto messenger = std::make_unique<NiceMock<MockIMessenger>>();
    NiceMock<MockIMessenger>* messengerRaw = messenger.get();

    EXPECT_CALL(*messengerRaw, sendSubscribeMessage(_)).Times(1);

    v2::CpuCom cpucom{std::move(messenger)};

    cpucom.onNotification(m_testNotificationCommand, m_testNotificationMessageData);

    cpucom.subscribe(m_testNotificationCommand, m_testCommandCallback);
    cpucom.onNotification(m_testNotificationCommand, m_testNotificationMessageData);
}

TEST_F(libCpuComV2Test, onSendCommandResultTest)
{
    constexpr int result = 0;
    auto messenger = std::make_unique<NiceMock<MockIMessenger>>();

    v2::CpuCom cpucom{std::move(messenger)};

    cpucom.onSendCommandResult(m_testSendResultCommand, result);

    cpucom.initialize(m_testSendCommandErrorCallback, m_testConnectionClosedCallback);
    cpucom.onSendCommandResult(m_testSendResultCommand, result);
}

TEST_F(libCpuComV2Test, onRequestResponseTest)
{
    auto messenger = std::make_unique<NiceMock<MockUUIDMessenger>>();
    NiceMock<MockUUIDMessenger>* messengerRaw = messenger.get();

    common::UUID id;
    std::vector<uint8_t> data;

    EXPECT_CALL(*messengerRaw, sendRequestMessage(_, _, _, _)).Times(2);

    v2::CpuCom cpucom{std::move(messenger)};

    auto response1 =
        cpucom.request(m_testRequestCommand1, m_testRequestMessageData, m_testResponseCommand);
    cpucom.onRequestResponse(id, data);

    auto response2 =
        cpucom.request(m_testRequestCommand2, m_testRequestMessageData, m_testResponseCommand);
    id = messengerRaw->getRequestUuid();
    cpucom.onRequestResponse(id, data);
}

TEST_F(libCpuComV2Test, onDeliveryStatusTest)
{
    auto messenger = std::make_unique<NiceMock<MockUUIDMessenger>>();
    NiceMock<MockUUIDMessenger>* messengerRaw = messenger.get();

    constexpr bool status = true;
    common::UUID id;

    EXPECT_CALL(*messengerRaw, sendSendCommandWithDeliveryStatusMessage(_, _, _)).Times(2);

    v2::CpuCom cpucom{std::move(messenger)};

    cpucom.send(m_testSendCommandWithDeliveryStatusCommand1,
                m_testSendCommandWithDeliveryStatusMessageData, m_testDeliveryStatusCallback);
    cpucom.onDeliveryStatus(id, status);

    cpucom.send(m_testSendCommandWithDeliveryStatusCommand2,
                m_testSendCommandWithDeliveryStatusMessageData, m_testDeliveryStatusCallback);
    id = messengerRaw->getSendUuid();
    cpucom.onDeliveryStatus(id, status);
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
