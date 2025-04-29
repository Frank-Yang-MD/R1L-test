/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include <iterator>
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "impl_CpuComImpl.h"

#include "CpuComMessage.h"
#include "Error.h"
#include "Pack.h"
#include "ThreadPool.h"
#include "common/MockIoDevice.h"
#include "libCpuCom.h"
#include "libMelcoCommon.h"
#include "mock/mock_Socket.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

using common::CpuCommand;
using common::mock_Socket;
using common::pack;
using common::unpack;

using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::SetArgPointee;

class SendSameThreadPolicy : public common::SameThreadThreadPolicy {
};
class ReceiveSameThreadPolicy : public common::SameThreadThreadPolicy {
};

class DefaultCpuCommandListener : public ICpuCommandListener {
public:
    virtual ~DefaultCpuCommandListener() = default;
    virtual void onReceiveCommand(const CpuCommand& command,
                                  const std::vector<uint8_t>& data) override
    {
        m_command = command;
        m_data = data;
    }

public:
    CpuCommand m_command;
    std::vector<uint8_t> m_data;
};

class DefaultCpuCommandErrorListener : public ICpuCommandErrorListener {
public:
    virtual ~DefaultCpuCommandErrorListener() = default;
    virtual void onError(const CpuCommand& command, int errorCode) override
    {
        m_command = command;
        m_errorCode = errorCode;
    }

public:
    CpuCommand m_command;
    int m_errorCode;
};

class libCpuComTest : public ::testing::Test {
public:
    libCpuComTest()
        : m_testSubscribeCommand1(std::make_pair(0x01, 0x07))
        , m_testSubscribeMessage1(CpuComId::Subscribe,
                                  pack(m_testSubscribeCommand1, std::vector<uint8_t>()))
        , m_testSubscribeCommand2(std::make_pair(0x02, 0x04))
        , m_testSubscribeMessage2(CpuComId::Subscribe,
                                  pack(m_testSubscribeCommand2, std::vector<uint8_t>()))
        , m_testUnsubscribeCommand1(std::make_pair(0x01, 0x07))
        , m_testUnsubscribeMessage1(CpuComId::Unsubscribe,
                                    pack(m_testUnsubscribeCommand1, std::vector<uint8_t>()))
        , m_testUnsubscribeCommand2(std::make_pair(0x02, 0x04))
        , m_testUnsubscribeMessage2(CpuComId::Unsubscribe,
                                    pack(m_testUnsubscribeCommand2, std::vector<uint8_t>()))
        , m_testSendCommand1(std::make_pair(0x01, 0x01))
        , m_testSendMessageData1({0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07})
        , m_testSendMessage1(CpuComId::SendCommand,
                             pack(m_testSendCommand1, m_testSendMessageData1))
        , m_testUnsupportedCommand1(std::make_pair(0x07, 0x07))
        , m_testUnsupportedMessage1(CpuComId::DeliveryStatus,
                                    pack(m_testUnsupportedCommand1, std::vector<uint8_t>()))
        , m_testCommandResultCommand(std::make_pair(0x30, 0x40))
        , m_testCommandResultMessage(CpuComId::SendCommandResult,
                                     pack(m_testCommandResultCommand, common::ERR_BUSY))
        , m_testCommandResultSizeNotCorrectMessage(
              CpuComId::SendCommandResult,
              pack(m_testCommandResultCommand, std::vector<uint8_t>()))
        , m_testNotificationCommand1(std::make_pair(0x13, 0x31))
        , m_testNotificationMessage1(
              CpuComId::Notification,
              pack(m_testNotificationCommand1, std::vector<uint8_t>({0x01, 0x02, 0x03, 0x04})))
        , m_testNotificationCommand2(std::make_pair(0x02, 0x04))
        , m_testNotificationData2({0x10, 0x20, 0x30, 0x40})
        , m_testNotificationMessage2(CpuComId::Notification,
                                     pack(m_testNotificationCommand2, m_testNotificationData2))
        , m_testNotificationCommandSizeNotCorrectMessage(
              CpuComId::Notification,
              pack(m_testNotificationCommand1, std::vector<uint8_t>()))
        , m_testSubscribeCommandsList({m_testSubscribeCommand1, m_testSubscribeCommand1,
                                       m_testSubscribeCommand1, m_testSubscribeCommand2})
        , m_testUnsubscribeCommandsList({std::make_pair(1, 1), m_testSubscribeCommand2,
                                         m_testSubscribeCommand1, std::make_pair(0, 0),
                                         m_testSubscribeCommand1, m_testSubscribeCommand1})
    {
        m_serializedSubscribeMessage1 = m_testSubscribeMessage1.serialize();
        m_serializedSubscribeMessage2 = m_testSubscribeMessage2.serialize();
        m_serializedUnsubscribeMessage1 = m_testUnsubscribeMessage1.serialize();
        m_serializedUnsubscribeMessage2 = m_testUnsubscribeMessage2.serialize();
        m_serializedSendMessage1 = m_testSendMessage1.serialize();

        m_serializedUnsupportedCommandMessage1 = m_testUnsupportedMessage1.serialize();
        m_serializedTestCommandResult = m_testCommandResultMessage.serialize();
        m_serializedTestCommandResultSizeNotCorrect =
            m_testCommandResultSizeNotCorrectMessage.serialize();

        m_serializedTestNotificationMessage1 = m_testNotificationMessage1.serialize();
        m_serializedTestNotificationMessage2 = m_testNotificationMessage2.serialize();
        m_serializedTestNotificationCommandSizeNotCorrect =
            m_testNotificationCommandSizeNotCorrectMessage.serialize();
    }

    ~libCpuComTest() {}

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

    CpuCommand m_testSubscribeCommand1;
    CpuComMessage m_testSubscribeMessage1;
    std::vector<uint8_t> m_serializedSubscribeMessage1;

    CpuCommand m_testSubscribeCommand2;
    CpuComMessage m_testSubscribeMessage2;
    std::vector<uint8_t> m_serializedSubscribeMessage2;

    CpuCommand m_testUnsubscribeCommand1;
    CpuComMessage m_testUnsubscribeMessage1;
    std::vector<uint8_t> m_serializedUnsubscribeMessage1;

    CpuCommand m_testUnsubscribeCommand2;
    CpuComMessage m_testUnsubscribeMessage2;
    std::vector<uint8_t> m_serializedUnsubscribeMessage2;

    CpuCommand m_testSendCommand1;
    std::vector<uint8_t> m_testSendMessageData1;
    CpuComMessage m_testSendMessage1;
    std::vector<uint8_t> m_serializedSendMessage1;

    CpuCommand m_testUnsupportedCommand1;
    CpuComMessage m_testUnsupportedMessage1;
    std::vector<uint8_t> m_serializedUnsupportedCommandMessage1;

    CpuCommand m_testCommandResultCommand;
    CpuComMessage m_testCommandResultMessage;
    std::vector<uint8_t> m_serializedTestCommandResult;

    CpuComMessage m_testCommandResultSizeNotCorrectMessage;
    std::vector<uint8_t> m_serializedTestCommandResultSizeNotCorrect;

    CpuCommand m_testNotificationCommand1;
    CpuComMessage m_testNotificationMessage1;
    std::vector<uint8_t> m_serializedTestNotificationMessage1;

    CpuCommand m_testNotificationCommand2;
    std::vector<uint8_t> m_testNotificationData2;
    CpuComMessage m_testNotificationMessage2;
    std::vector<uint8_t> m_serializedTestNotificationMessage2;

    CpuComMessage m_testNotificationCommandSizeNotCorrectMessage;
    std::vector<uint8_t> m_serializedTestNotificationCommandSizeNotCorrect;

    std::list<CpuCommand> m_testSubscribeCommandsList;
    std::list<CpuCommand> m_testUnsubscribeCommandsList;
};

TEST_F(libCpuComTest, notificationThreadNotStartedTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);

    MockIoDevice* d = new MockIoDevice();
    std::unique_ptr<IIoDevice> device(d);

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*s, getFileDescriptor()).Times(AtLeast(1)).WillRepeatedly(Return(1));

    EXPECT_CALL(*d, eventfd(0, 0)).Times(1).WillOnce(Return(-1));

    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));
}

TEST_F(libCpuComTest, subscribeTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);

    MockIoDevice* d = new MockIoDevice();
    std::unique_ptr<IIoDevice> device(d);

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*s, getFileDescriptor()).Times(AtLeast(1)).WillRepeatedly(Return(1));
    EXPECT_CALL(*s, write(m_serializedSubscribeMessage1)).Times(1);
    EXPECT_CALL(*s, write(m_serializedSubscribeMessage2)).Times(1);

    EXPECT_CALL(*d, eventfd(0, 0)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, pollInit(_, _)).Times(1);
    EXPECT_CALL(*d, poll()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, isSocketFdReceivePollInEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isSocketFdReceivePollHupEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isStopFdReceivePollInEvent()).Times(1).WillOnce(Return(true));

    std::unique_ptr<DefaultCpuCommandListener> listener(new DefaultCpuCommandListener());
    std::unique_ptr<DefaultCpuCommandErrorListener> errorCallback(
        new DefaultCpuCommandErrorListener());
    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));

    libCpuComImpl->setErrorCallback(errorCallback.get());
    libCpuComImpl->subscribe(m_testSubscribeCommand1, listener.get());
    libCpuComImpl->subscribe(m_testSubscribeCommand1, listener.get());
    libCpuComImpl->subscribe(m_testSubscribeCommand1, listener.get());
    libCpuComImpl->subscribe(m_testSubscribeCommand2, listener.get());
}

TEST_F(libCpuComTest, subscribeWhenSocketInvalidTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);
    std::unique_ptr<IIoDevice> device(new MockIoDevice());

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(false));

    std::unique_ptr<DefaultCpuCommandListener> listener(new DefaultCpuCommandListener());
    std::unique_ptr<DefaultCpuCommandErrorListener> errorCallback(
        new DefaultCpuCommandErrorListener());
    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));

    libCpuComImpl->setErrorCallback(errorCallback.get());
    libCpuComImpl->subscribe(m_testSubscribeCommand1, listener.get());
}

TEST_F(libCpuComTest, unsubscribeTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);

    MockIoDevice* d = new MockIoDevice();
    std::unique_ptr<IIoDevice> device(d);

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*s, getFileDescriptor()).Times(AtLeast(1)).WillRepeatedly(Return(1));
    EXPECT_CALL(*s, write(m_serializedSubscribeMessage1)).Times(1);
    EXPECT_CALL(*s, write(m_serializedSubscribeMessage2)).Times(1);
    EXPECT_CALL(*s, write(m_serializedUnsubscribeMessage2)).Times(1);
    EXPECT_CALL(*s, write(m_serializedUnsubscribeMessage1)).Times(1);

    EXPECT_CALL(*d, eventfd(0, 0)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, pollInit(_, _)).Times(1);
    EXPECT_CALL(*d, poll()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, isSocketFdReceivePollInEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isSocketFdReceivePollHupEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isStopFdReceivePollInEvent()).Times(1).WillOnce(Return(true));

    std::unique_ptr<DefaultCpuCommandListener> listener(new DefaultCpuCommandListener());
    std::unique_ptr<DefaultCpuCommandErrorListener> errorCallback(
        new DefaultCpuCommandErrorListener());
    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));

    libCpuComImpl->setErrorCallback(errorCallback.get());
    libCpuComImpl->subscribe(m_testSubscribeCommand1, listener.get());
    libCpuComImpl->subscribe(m_testSubscribeCommand1, listener.get());
    libCpuComImpl->unsubscribe(std::make_pair(1, 1), listener.get());
    libCpuComImpl->subscribe(m_testSubscribeCommand1, listener.get());
    libCpuComImpl->subscribe(m_testSubscribeCommand2, listener.get());
    libCpuComImpl->unsubscribe(m_testSubscribeCommand2, listener.get());
    libCpuComImpl->unsubscribe(m_testSubscribeCommand1, listener.get());
    libCpuComImpl->unsubscribe(m_testSubscribeCommand1, listener.get());
    libCpuComImpl->unsubscribe(m_testSubscribeCommand1, listener.get());
    libCpuComImpl->unsubscribe(std::make_pair(0, 0), listener.get());
}

TEST_F(libCpuComTest, unsubscribeWhenSocketInvalidTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);
    std::unique_ptr<IIoDevice> device(new MockIoDevice());

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(false));

    std::unique_ptr<DefaultCpuCommandListener> listener(new DefaultCpuCommandListener());
    std::unique_ptr<DefaultCpuCommandErrorListener> errorCallback(
        new DefaultCpuCommandErrorListener());
    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));

    libCpuComImpl->setErrorCallback(errorCallback.get());
    libCpuComImpl->subscribe(m_testSubscribeCommand1, listener.get());
    libCpuComImpl->unsubscribe(m_testSubscribeCommand1, listener.get());
}

TEST_F(libCpuComTest, subscribeListTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);

    MockIoDevice* d = new MockIoDevice();
    std::unique_ptr<IIoDevice> device(d);

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*s, getFileDescriptor()).Times(AtLeast(1)).WillRepeatedly(Return(1));
    EXPECT_CALL(*s, write(m_serializedSubscribeMessage1)).Times(1);
    EXPECT_CALL(*s, write(m_serializedSubscribeMessage2)).Times(1);

    EXPECT_CALL(*d, eventfd(0, 0)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, pollInit(_, _)).Times(1);
    EXPECT_CALL(*d, poll()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, isSocketFdReceivePollInEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isSocketFdReceivePollHupEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isStopFdReceivePollInEvent()).Times(1).WillOnce(Return(true));

    std::unique_ptr<DefaultCpuCommandListener> listener(new DefaultCpuCommandListener());
    std::unique_ptr<DefaultCpuCommandErrorListener> errorCallback(
        new DefaultCpuCommandErrorListener());
    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));

    libCpuComImpl->setErrorCallback(errorCallback.get());
    libCpuComImpl->subscribe(std::list<CpuCommand>(), listener.get());
    libCpuComImpl->subscribe(m_testSubscribeCommandsList, listener.get());
}

TEST_F(libCpuComTest, unsubscribeListTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);

    MockIoDevice* d = new MockIoDevice();
    std::unique_ptr<IIoDevice> device(d);

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*s, getFileDescriptor()).Times(AtLeast(1)).WillRepeatedly(Return(1));
    EXPECT_CALL(*s, write(m_serializedSubscribeMessage1)).Times(1);
    EXPECT_CALL(*s, write(m_serializedSubscribeMessage2)).Times(1);
    EXPECT_CALL(*s, write(m_serializedUnsubscribeMessage1)).Times(1);

    EXPECT_CALL(*d, eventfd(0, 0)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, pollInit(_, _)).Times(1);
    EXPECT_CALL(*d, poll()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, isSocketFdReceivePollInEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isSocketFdReceivePollHupEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isStopFdReceivePollInEvent()).Times(1).WillOnce(Return(true));

    std::unique_ptr<DefaultCpuCommandListener> listener(new DefaultCpuCommandListener());
    std::unique_ptr<DefaultCpuCommandListener> listener2(new DefaultCpuCommandListener());
    std::unique_ptr<DefaultCpuCommandErrorListener> errorCallback(
        new DefaultCpuCommandErrorListener());
    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));

    libCpuComImpl->setErrorCallback(errorCallback.get());
    libCpuComImpl->subscribe(m_testSubscribeCommand1, listener.get());
    libCpuComImpl->subscribe(m_testSubscribeCommand2, listener2.get());
    libCpuComImpl->unsubscribe(m_testUnsubscribeCommandsList, listener.get());
    libCpuComImpl->unsubscribe(std::list<CpuCommand>(), listener.get());
}

TEST_F(libCpuComTest, sendTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);

    MockIoDevice* d = new MockIoDevice();
    std::unique_ptr<IIoDevice> device(d);

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*s, getFileDescriptor()).Times(AtLeast(1)).WillRepeatedly(Return(1));
    EXPECT_CALL(*s, write(m_serializedSendMessage1)).Times(3);

    EXPECT_CALL(*d, eventfd(0, 0)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, pollInit(_, _)).Times(1);
    EXPECT_CALL(*d, poll()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, isSocketFdReceivePollInEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isSocketFdReceivePollHupEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isStopFdReceivePollInEvent()).Times(1).WillOnce(Return(true));

    std::unique_ptr<DefaultCpuCommandListener> listener(new DefaultCpuCommandListener());
    std::unique_ptr<DefaultCpuCommandErrorListener> errorCallback(
        new DefaultCpuCommandErrorListener());
    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));

    libCpuComImpl->setErrorCallback(errorCallback.get());
    libCpuComImpl->send(m_testSendCommand1, m_testSendMessageData1);
    libCpuComImpl->send(m_testSendCommand1, m_testSendMessageData1);
    libCpuComImpl->send(m_testSendCommand1, m_testSendMessageData1);
}

TEST_F(libCpuComTest, sendWhenSocketInvalidTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);

    MockIoDevice* d = new MockIoDevice();
    std::unique_ptr<IIoDevice> device(d);

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(false));

    std::unique_ptr<DefaultCpuCommandListener> listener(new DefaultCpuCommandListener());
    std::unique_ptr<DefaultCpuCommandErrorListener> errorCallback(
        new DefaultCpuCommandErrorListener());
    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));

    libCpuComImpl->setErrorCallback(errorCallback.get());
    libCpuComImpl->send(m_testSendCommand1, m_testSendMessageData1);
}

TEST_F(libCpuComTest, readFromDaemonUnsupportedCommandTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);

    MockIoDevice* d = new MockIoDevice();
    std::unique_ptr<IIoDevice> device(d);

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*s, getFileDescriptor()).Times(AtLeast(1)).WillRepeatedly(Return(1));
    EXPECT_CALL(*s, readAll()).Times(1).WillOnce(Return(m_serializedUnsupportedCommandMessage1));

    EXPECT_CALL(*d, eventfd(0, 0)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, pollInit(_, _)).Times(1);
    EXPECT_CALL(*d, poll()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, isSocketFdReceivePollInEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isSocketFdReceivePollHupEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isStopFdReceivePollInEvent()).Times(1).WillOnce(Return(true));

    std::unique_ptr<DefaultCpuCommandListener> listener(new DefaultCpuCommandListener());
    std::unique_ptr<DefaultCpuCommandErrorListener> errorCallback(
        new DefaultCpuCommandErrorListener());
    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));
    libCpuComImpl->setErrorCallback(errorCallback.get());

    libCpuComImpl->onReadyReadFromDaemon();
}

TEST_F(libCpuComTest, onSendCommandResultTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);

    MockIoDevice* d = new MockIoDevice();
    std::unique_ptr<IIoDevice> device(d);

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*s, getFileDescriptor()).Times(AtLeast(1)).WillRepeatedly(Return(1));
    EXPECT_CALL(*s, readAll())
        .Times(3)
        .WillOnce(Return(m_serializedTestCommandResult))
        .WillOnce(Return(m_serializedTestCommandResult))
        .WillOnce(Return(m_serializedTestCommandResultSizeNotCorrect));

    EXPECT_CALL(*d, eventfd(0, 0)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, pollInit(_, _)).Times(1);
    EXPECT_CALL(*d, poll()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, isSocketFdReceivePollInEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isSocketFdReceivePollHupEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isStopFdReceivePollInEvent()).Times(1).WillOnce(Return(true));

    std::unique_ptr<DefaultCpuCommandListener> listener(new DefaultCpuCommandListener());
    std::unique_ptr<DefaultCpuCommandErrorListener> errorCallback(
        new DefaultCpuCommandErrorListener());
    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));
    libCpuComImpl->onReadyReadFromDaemon();

    libCpuComImpl->setErrorCallback(errorCallback.get());
    libCpuComImpl->onReadyReadFromDaemon();
    EXPECT_EQ(errorCallback->m_command, this->m_testCommandResultCommand);
    EXPECT_EQ(errorCallback->m_errorCode, common::ERR_BUSY);

    libCpuComImpl->onReadyReadFromDaemon();
    EXPECT_EQ(errorCallback->m_command, this->m_testCommandResultCommand);
}

TEST_F(libCpuComTest, onNotificationTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);

    MockIoDevice* d = new MockIoDevice();
    std::unique_ptr<IIoDevice> device(d);

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*s, getFileDescriptor()).Times(AtLeast(1)).WillRepeatedly(Return(1));
    EXPECT_CALL(*s, write(m_serializedSubscribeMessage2)).Times(1);
    EXPECT_CALL(*s, readAll())
        .Times(2)
        .WillOnce(Return(m_serializedTestNotificationMessage2))
        .WillOnce(Return(m_serializedTestNotificationMessage1));

    EXPECT_CALL(*d, eventfd(0, 0)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, pollInit(_, _)).Times(1);
    EXPECT_CALL(*d, poll()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, isSocketFdReceivePollInEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isSocketFdReceivePollHupEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isStopFdReceivePollInEvent()).Times(1).WillOnce(Return(true));

    std::unique_ptr<DefaultCpuCommandListener> listener(new DefaultCpuCommandListener());
    std::unique_ptr<DefaultCpuCommandErrorListener> errorCallback(
        new DefaultCpuCommandErrorListener());
    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));

    libCpuComImpl->setErrorCallback(errorCallback.get());
    libCpuComImpl->subscribe(m_testSubscribeCommand2, listener.get());
    libCpuComImpl->onReadyReadFromDaemon();
    libCpuComImpl->onReadyReadFromDaemon();

    bool isEqual = std::equal(m_testNotificationData2.begin(), m_testNotificationData2.end(),
                              listener->m_data.begin());
    EXPECT_EQ(isEqual, true);
    EXPECT_EQ(listener->m_command, this->m_testNotificationCommand2);
}

TEST_F(libCpuComTest, startNotificationThreadWhenPollingFailTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);

    MockIoDevice* d = new MockIoDevice();
    std::unique_ptr<IIoDevice> device(d);

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*s, getFileDescriptor()).Times(AtLeast(1)).WillRepeatedly(Return(1));

    EXPECT_CALL(*d, eventfd(0, 0)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, pollInit(_, _)).Times(1);
    EXPECT_CALL(*d, poll()).Times(2).WillOnce(Return(-1)).WillOnce(Return(0));
    EXPECT_CALL(*d, isSocketFdReceivePollInEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isSocketFdReceivePollHupEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isStopFdReceivePollInEvent()).Times(1).WillOnce(Return(true));

    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));
}

TEST_F(libCpuComTest, startNotificationThreadWhenSocketFdReceivePollInTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);

    MockIoDevice* d = new MockIoDevice();
    std::unique_ptr<IIoDevice> device(d);

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*s, getFileDescriptor()).Times(AtLeast(1)).WillRepeatedly(Return(1));
    EXPECT_CALL(*s, readAll()).Times(1).WillOnce(Return(std::vector<uint8_t>()));

    EXPECT_CALL(*d, eventfd(0, 0)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, pollInit(_, _)).Times(1);
    EXPECT_CALL(*d, poll()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, isSocketFdReceivePollInEvent()).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*d, isStopFdReceivePollInEvent()).Times(1).WillOnce(Return(true));

    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));
}

TEST_F(libCpuComTest, startNotificationThreadWhenSocketFdReceivePollHupTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);

    MockIoDevice* d = new MockIoDevice();
    std::unique_ptr<IIoDevice> device(d);

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*s, getFileDescriptor()).Times(AtLeast(1)).WillRepeatedly(Return(1));

    EXPECT_CALL(*d, eventfd(0, 0)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, pollInit(_, _)).Times(1);
    EXPECT_CALL(*d, poll()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, isSocketFdReceivePollInEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isSocketFdReceivePollHupEvent()).Times(1).WillOnce(Return(true));

    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));
}

TEST_F(libCpuComTest, startNotificationThreadWhenStopFdReceivePollHupTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);

    MockIoDevice* d = new MockIoDevice();
    std::unique_ptr<IIoDevice> device(d);

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*s, getFileDescriptor()).Times(AtLeast(1)).WillRepeatedly(Return(1));

    EXPECT_CALL(*d, eventfd(0, 0)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, pollInit(_, _)).Times(1);
    EXPECT_CALL(*d, poll()).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, isSocketFdReceivePollInEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isSocketFdReceivePollHupEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isStopFdReceivePollInEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isStopFdReceivePollHupEvent()).Times(1).WillOnce(Return(true));

    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));
}

TEST_F(libCpuComTest, startNotificationThreadWhenSocketFdReceivePollInTwiceTimesTest)
{
    mock_Socket* s = new mock_Socket();
    std::unique_ptr<common::Socket> socket(s);

    MockIoDevice* d = new MockIoDevice();
    std::unique_ptr<IIoDevice> device(d);

    EXPECT_CALL(*s, connect(_)).Times(1);
    EXPECT_CALL(*s, valid()).Times(AtLeast(1)).WillRepeatedly(Return(true));
    EXPECT_CALL(*s, getFileDescriptor()).Times(AtLeast(1)).WillRepeatedly(Return(1));

    EXPECT_CALL(*d, eventfd(0, 0)).Times(1).WillOnce(Return(0));
    EXPECT_CALL(*d, pollInit(_, _)).Times(1);
    EXPECT_CALL(*d, poll()).Times(2).WillRepeatedly(Return(0));
    EXPECT_CALL(*d, isSocketFdReceivePollInEvent()).Times(2).WillRepeatedly(Return(false));
    EXPECT_CALL(*d, isSocketFdReceivePollHupEvent())
        .Times(2)
        .WillOnce(Return(false))
        .WillOnce(Return(true));
    EXPECT_CALL(*d, isStopFdReceivePollInEvent()).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*d, isStopFdReceivePollHupEvent()).Times(1).WillOnce(Return(false));

    std::unique_ptr<CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>> libCpuComImpl(
        new CpuComImpl<SendSameThreadPolicy, ReceiveSameThreadPolicy>(std::move(socket),
                                                                      std::move(device)));
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
