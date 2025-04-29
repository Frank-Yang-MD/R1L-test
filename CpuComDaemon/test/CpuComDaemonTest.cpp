/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "CpuComDaemon.h"
#include "CPUCommon.h"

#include "MockICPU.h"
#include "MockIMessageServer.h"
#include "MockMutexWrapper.h"
#include <mock/mock_IPeriodicTaskExecutor.h>

#include <gtest/gtest.h>
#include <memory>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

using ::testing::_;
using ::testing::An;
using ::testing::ByMove;
using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SetArgReferee;
using ::testing::Test;

class CpuComDaemonTest : public Test {
protected:
    CpuComDaemonTest();
    ~CpuComDaemonTest();

    common::CpuCommand mSubscribeCommand;
    common::CpuCommand mRequestCommand;
    common::CpuCommand mResponseCommand;
    common::CpuCommand mSendCommand;
    common::CpuCommand mSendCommandWithDeliveryStatus;
    SessionID mSessionSubscribeId;
    SessionID mSessionRequestId;
    SessionID mSessionConnectId;
    SessionID mSessionSendId;
    SessionID mSessionSendWithDeliveryStatusId;

    common::UUID mRequestUUID;
    common::UUID mSendUUID;

    std::vector<uint8_t> mSubscribeRawData;
    std::vector<uint8_t> mRequestRawData;
    std::vector<uint8_t> mSendRawData;

    std::pair<common::CpuCommand, std::vector<uint8_t>> mSubscribeData;
    std::pair<common::CpuCommand, std::vector<uint8_t>> mRequestData;
    std::pair<common::CpuCommand, std::vector<uint8_t>> mResponseData;

    std::function<void(void)> mTaskCallable;
    std::function<bool(void)> mPredicateCallable;

    std::promise<void> p;
};

CpuComDaemonTest::CpuComDaemonTest()
    : mSubscribeCommand(std::make_pair(0x01, 0x01))
    , mRequestCommand(std::make_pair(0x02, 0x02))
    , mResponseCommand(std::make_pair(0x03, 0x03))
    , mSendCommand(std::make_pair(0x04, 0x04))
    , mSendCommandWithDeliveryStatus(std::make_pair(0x05, 0x05))
    , mSessionSubscribeId("sessionSubscribeId")
    , mSessionRequestId("sessionRequestId")
    , mSessionConnectId("mSessionConnectId")
    , mSessionSendId("sessionSendId")
    , mSessionSendWithDeliveryStatusId("sessionSendWithDeliveryStatusId")
    , mRequestUUID("00000000-0000-0000-0000-000000000000")
    , mSendUUID("00000001-0001-0001-0001-000000000001")
    , mSubscribeRawData{0x00, 0x00}
    , mRequestRawData{0x01, 0x01}
    , mSendRawData{0x01, 0x01}
    , mSubscribeData{mSubscribeCommand, mSubscribeRawData}
    , mRequestData{mRequestCommand, mRequestRawData}
    , mResponseData{mResponseCommand, mRequestRawData}
{
}

CpuComDaemonTest::~CpuComDaemonTest() {}

TEST_F(CpuComDaemonTest, vcpuInitializeFailedTest)
{
    auto messageServer = std::make_unique<NiceMock<MockIMessageServer>>();
    NiceMock<MockIMessageServer>* messageServerRaw = messageServer.get();
    auto vcpu = std::make_unique<NiceMock<MockICPU>>();
    NiceMock<MockICPU>* vcpuRaw = vcpu.get();
    auto periodicExecutor = std::make_unique<NiceMock<common::mock_IPeriodicTaskExecutor>>();
    auto subscribersMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();
    auto requestsMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();

    CpuComDaemon daemon{std::move(messageServer), std::move(vcpu), std::move(periodicExecutor),
                        std::move(subscribersMutexWrapper), std::move(requestsMutexWrapper)};

    EXPECT_CALL(*vcpuRaw, initialize()).WillOnce(Return(false));
    EXPECT_CALL(*messageServerRaw, initialize(An<IMessageServer::OnNewConnectionHandler>(),
                                              An<IMessageServer::OnConnectionClosedHandler>()));
    EXPECT_CALL(*messageServerRaw,
                setSendCommandMessageHandler(An<IMessageServer::OnSendCommandHandler>(), &daemon));
    EXPECT_CALL(*messageServerRaw,
                setSubscribeMessageHandler(An<IMessageServer::OnSubscribeHandler>(), &daemon));
    EXPECT_CALL(*messageServerRaw,
                setUnsubscribeMessageHandler(An<IMessageServer::OnUnsubscribeHandler>(), &daemon));
    EXPECT_CALL(*messageServerRaw,
                setRequestMessageHandler(An<IMessageServer::OnRequestHandler>(), &daemon));
    EXPECT_CALL(*messageServerRaw, setCancelRequestMessageHandler(
                                       An<IMessageServer::OnCancelRequestHandler>(), &daemon));
    EXPECT_CALL(*messageServerRaw,
                setSendCommandWithDeliveryStatusMessageHandler(
                    An<IMessageServer::OnSendCommandWithDeliveryStatusHandler>(), &daemon));
    daemon.start();
}

TEST_F(CpuComDaemonTest, isDaemonStoppedTest)
{
    auto messageServer = std::make_unique<NiceMock<MockIMessageServer>>();
    auto vcpu = std::make_unique<NiceMock<MockICPU>>();
    NiceMock<MockICPU>* vcpuRaw = vcpu.get();
    auto periodicExecutor = std::make_unique<NiceMock<common::mock_IPeriodicTaskExecutor>>();
    NiceMock<common::mock_IPeriodicTaskExecutor>* periodicExecutorRaw = periodicExecutor.get();
    auto subscribersMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();
    auto requestsMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();

    CpuComDaemon daemon{std::move(messageServer), std::move(vcpu), std::move(periodicExecutor),
                        std::move(subscribersMutexWrapper), std::move(requestsMutexWrapper)};

    EXPECT_CALL(*vcpuRaw, initialize()).WillOnce(Return(true));
    EXPECT_CALL(*periodicExecutorRaw, submit(_, _))
        .WillOnce(DoAll(SaveArg<0>(&mTaskCallable), SaveArg<1>(&mPredicateCallable),
                        Return(ByMove(p.get_future()))));

    daemon.start();
    mPredicateCallable();
}

TEST_F(CpuComDaemonTest, handleSubscribeMessageTest)
{
    auto messageServer = std::make_unique<NiceMock<MockIMessageServer>>();
    NiceMock<MockIMessageServer>* messageServerRaw = messageServer.get();
    auto vcpu = std::make_unique<NiceMock<MockICPU>>();
    NiceMock<MockICPU>* vcpuRaw = vcpu.get();
    auto periodicExecutor = std::make_unique<NiceMock<common::mock_IPeriodicTaskExecutor>>();
    NiceMock<common::mock_IPeriodicTaskExecutor>* periodicExecutorRaw = periodicExecutor.get();
    auto subscribersMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();
    auto requestsMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();

    CpuComDaemon daemon{std::move(messageServer), std::move(vcpu), std::move(periodicExecutor),
                        std::move(subscribersMutexWrapper), std::move(requestsMutexWrapper)};

    EXPECT_CALL(*vcpuRaw, initialize()).WillOnce(Return(true));
    EXPECT_CALL(*periodicExecutorRaw, submit(_, _))
        .WillOnce(DoAll(SaveArg<0>(&mTaskCallable), SaveArg<1>(&mPredicateCallable),
                        Return(ByMove(p.get_future()))));
    ON_CALL(*vcpuRaw, read(_)).WillByDefault(DoAll(SetArgReferee<0>(mSubscribeData), Return(true)));
    EXPECT_CALL(*messageServerRaw,
                sendNotificationMessage(mSessionSubscribeId, mSubscribeCommand, mSubscribeRawData));

    daemon.onSubscribe(mSessionSubscribeId, mSubscribeCommand);
    daemon.start();
    mTaskCallable();
    daemon.onUnsubscribe(mSessionSubscribeId, mSubscribeCommand);
}

TEST_F(CpuComDaemonTest, handleSubscribeMessageFailedTest)
{
    auto messageServer = std::make_unique<NiceMock<MockIMessageServer>>();
    NiceMock<MockIMessageServer>* messageServerRaw = messageServer.get();
    auto vcpu = std::make_unique<NiceMock<MockICPU>>();
    NiceMock<MockICPU>* vcpuRaw = vcpu.get();
    auto periodicExecutor = std::make_unique<NiceMock<common::mock_IPeriodicTaskExecutor>>();
    NiceMock<common::mock_IPeriodicTaskExecutor>* periodicExecutorRaw = periodicExecutor.get();
    auto subscribersMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();
    auto requestsMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();

    CpuComDaemon daemon{std::move(messageServer), std::move(vcpu), std::move(periodicExecutor),
                        std::move(subscribersMutexWrapper), std::move(requestsMutexWrapper)};

    EXPECT_CALL(*vcpuRaw, initialize()).WillOnce(Return(true));
    EXPECT_CALL(*periodicExecutorRaw, submit(_, _))
        .WillOnce(DoAll(SaveArg<0>(&mTaskCallable), SaveArg<1>(&mPredicateCallable),
                        Return(ByMove(p.get_future()))));
    ON_CALL(*vcpuRaw, read(_))
        .WillByDefault(DoAll(SetArgReferee<0>(mSubscribeData), Return(false)));
    EXPECT_CALL(*messageServerRaw,
                sendNotificationMessage(mSessionSubscribeId, mSubscribeCommand, mSubscribeRawData))
        .Times(0);

    daemon.onSubscribe(mSessionSubscribeId, mSubscribeCommand);
    daemon.start();
    mTaskCallable();
    daemon.onUnsubscribe(mSessionSubscribeId, mSubscribeCommand);
}

TEST_F(CpuComDaemonTest, handleRequestMessageTest)
{
    auto messageServer = std::make_unique<NiceMock<MockIMessageServer>>();
    NiceMock<MockIMessageServer>* messageServerRaw = messageServer.get();
    auto vcpu = std::make_unique<NiceMock<MockICPU>>();
    NiceMock<MockICPU>* vcpuRaw = vcpu.get();
    auto periodicExecutor = std::make_unique<NiceMock<common::mock_IPeriodicTaskExecutor>>();
    NiceMock<common::mock_IPeriodicTaskExecutor>* periodicExecutorRaw = periodicExecutor.get();
    auto subscribersMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();
    auto requestsMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();

    CpuComDaemon daemon{std::move(messageServer), std::move(vcpu), std::move(periodicExecutor),
                        std::move(subscribersMutexWrapper), std::move(requestsMutexWrapper)};

    EXPECT_CALL(*vcpuRaw, initialize()).WillOnce(Return(true));
    EXPECT_CALL(*periodicExecutorRaw, submit(_, _))
        .WillOnce(DoAll(SaveArg<0>(&mTaskCallable), SaveArg<1>(&mPredicateCallable),
                        Return(ByMove(p.get_future()))));
    EXPECT_CALL(*vcpuRaw, read(_)).WillOnce(DoAll(SetArgReferee<0>(mResponseData), Return(true)));
    EXPECT_CALL(*vcpuRaw, write(mRequestCommand, mRequestRawData)).WillOnce(Return(true));
    EXPECT_CALL(*messageServerRaw,
                sendRequestResponseMessage(mSessionRequestId, mRequestUUID, mRequestRawData));

    daemon.onRequest(mSessionRequestId, mRequestUUID, mRequestCommand, mRequestRawData,
                     mResponseCommand);
    daemon.start();
    mTaskCallable();
    daemon.onCancelRequest(mSessionRequestId, mRequestUUID);
}

TEST_F(CpuComDaemonTest, handleClientConnectionMessageTest)
{
    auto messageServer = std::make_unique<NiceMock<MockIMessageServer>>();
    auto vcpu = std::make_unique<NiceMock<MockICPU>>();
    NiceMock<MockICPU>* vcpuRaw = vcpu.get();
    auto periodicExecutor = std::make_unique<NiceMock<common::mock_IPeriodicTaskExecutor>>();
    NiceMock<common::mock_IPeriodicTaskExecutor>* periodicExecutorRaw = periodicExecutor.get();
    auto subscribersMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();
    auto requestsMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();

    CpuComDaemon daemon{std::move(messageServer), std::move(vcpu), std::move(periodicExecutor),
                        std::move(subscribersMutexWrapper), std::move(requestsMutexWrapper)};

    EXPECT_CALL(*vcpuRaw, initialize()).WillOnce(Return(true));
    EXPECT_CALL(*periodicExecutorRaw, submit(_, _))
        .WillOnce(DoAll(SaveArg<0>(&mTaskCallable), SaveArg<1>(&mPredicateCallable),
                        Return(ByMove(p.get_future()))));
    ON_CALL(*vcpuRaw, read(_)).WillByDefault(DoAll(SetArgReferee<0>(mRequestData), Return(true)));

    daemon.onSubscribe(mSessionSubscribeId, mSubscribeCommand);
    daemon.onRequest(mSessionRequestId, mRequestUUID, mRequestCommand, mRequestRawData,
                     mResponseCommand);
    daemon.onClientConnected(mSessionConnectId);
    daemon.start();
    mTaskCallable();
    daemon.onClientDisconnected(mSessionConnectId);
    daemon.onCancelRequest(mSessionRequestId, mRequestUUID);
    daemon.onUnsubscribe(mSessionSubscribeId, mSubscribeCommand);
}

TEST_F(CpuComDaemonTest, handleSendCommandMessageSuccessfullyTest)
{
    auto messageServer = std::make_unique<NiceMock<MockIMessageServer>>();
    NiceMock<MockIMessageServer>* messageServerRaw = messageServer.get();
    auto vcpu = std::make_unique<NiceMock<MockICPU>>();
    NiceMock<MockICPU>* vcpuRaw = vcpu.get();
    auto periodicExecutor = std::make_unique<NiceMock<common::mock_IPeriodicTaskExecutor>>();
    NiceMock<common::mock_IPeriodicTaskExecutor>* periodicExecutorRaw = periodicExecutor.get();
    auto subscribersMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();
    auto requestsMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();

    CpuComDaemon daemon{std::move(messageServer), std::move(vcpu), std::move(periodicExecutor),
                        std::move(subscribersMutexWrapper), std::move(requestsMutexWrapper)};

    EXPECT_CALL(*vcpuRaw, initialize()).WillOnce(Return(true));
    EXPECT_CALL(*periodicExecutorRaw, submit(_, _))
        .WillOnce(DoAll(SaveArg<0>(&mTaskCallable), SaveArg<1>(&mPredicateCallable),
                        Return(ByMove(p.get_future()))));
    ON_CALL(*vcpuRaw, read(_)).WillByDefault(DoAll(SetArgReferee<0>(mRequestData), Return(true)));
    ON_CALL(*vcpuRaw, write(mSendCommand, mSendRawData)).WillByDefault(Return(true));
    EXPECT_CALL(*messageServerRaw,
                sendSendCommandResultMessage(mSessionSendId, mSendCommand, common::ERR_BUSY))
        .Times(0);

    daemon.onSendCommand(mSessionSendId, mSendCommand, mSendRawData);
    daemon.start();
    mTaskCallable();
}

TEST_F(CpuComDaemonTest, handleSendCommandMessageFailedTest)
{
    auto messageServer = std::make_unique<NiceMock<MockIMessageServer>>();
    NiceMock<MockIMessageServer>* messageServerRaw = messageServer.get();
    auto vcpu = std::make_unique<NiceMock<MockICPU>>();
    NiceMock<MockICPU>* vcpuRaw = vcpu.get();
    auto periodicExecutor = std::make_unique<NiceMock<common::mock_IPeriodicTaskExecutor>>();
    NiceMock<common::mock_IPeriodicTaskExecutor>* periodicExecutorRaw = periodicExecutor.get();
    auto subscribersMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();
    auto requestsMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();

    CpuComDaemon daemon{std::move(messageServer), std::move(vcpu), std::move(periodicExecutor),
                        std::move(subscribersMutexWrapper), std::move(requestsMutexWrapper)};

    EXPECT_CALL(*vcpuRaw, initialize()).WillOnce(Return(true));
    EXPECT_CALL(*periodicExecutorRaw, submit(_, _))
        .WillOnce(DoAll(SaveArg<0>(&mTaskCallable), SaveArg<1>(&mPredicateCallable),
                        Return(ByMove(p.get_future()))));
    ON_CALL(*vcpuRaw, read(_)).WillByDefault(DoAll(SetArgReferee<0>(mRequestData), Return(true)));
    ON_CALL(*vcpuRaw, write(mSendCommand, mSendRawData)).WillByDefault(Return(false));
    EXPECT_CALL(*messageServerRaw,
                sendSendCommandResultMessage(mSessionSendId, mSendCommand, common::ERR_BUSY));

    daemon.onSendCommand(mSessionSendId, mSendCommand, mSendRawData);
    daemon.start();
    mTaskCallable();
}

TEST_F(CpuComDaemonTest, handleSendCommandWithDeliveryStatusTest)
{
    auto messageServer = std::make_unique<NiceMock<MockIMessageServer>>();
    NiceMock<MockIMessageServer>* messageServerRaw = messageServer.get();
    auto vcpu = std::make_unique<NiceMock<MockICPU>>();
    NiceMock<MockICPU>* vcpuRaw = vcpu.get();
    auto periodicExecutor = std::make_unique<NiceMock<common::mock_IPeriodicTaskExecutor>>();
    NiceMock<common::mock_IPeriodicTaskExecutor>* periodicExecutorRaw = periodicExecutor.get();
    auto subscribersMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();
    auto requestsMutexWrapper = std::make_unique<NiceMock<MockMutexWrapper>>();

    CpuComDaemon daemon{std::move(messageServer), std::move(vcpu), std::move(periodicExecutor),
                        std::move(subscribersMutexWrapper), std::move(requestsMutexWrapper)};

    EXPECT_CALL(*vcpuRaw, initialize()).WillOnce(Return(true));
    EXPECT_CALL(*periodicExecutorRaw, submit(_, _))
        .WillOnce(DoAll(SaveArg<0>(&mTaskCallable), SaveArg<1>(&mPredicateCallable),
                        Return(ByMove(p.get_future()))));
    ON_CALL(*vcpuRaw, read(_)).WillByDefault(DoAll(SetArgReferee<0>(mRequestData), Return(true)));
    ON_CALL(*vcpuRaw, write(mSendCommand, mSendRawData)).WillByDefault(Return(true));
    ON_CALL(*messageServerRaw,
            sendDeliveryStatusMessage(mSessionSendWithDeliveryStatusId, mSendUUID, true));

    daemon.onSendCommandWithDeliveryStatus(mSessionSendWithDeliveryStatusId, mSendUUID,
                                           mSendCommandWithDeliveryStatus, mSendRawData);
    daemon.start();
    mTaskCallable();
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
