/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Checksum.h"

#include "CpuComDaemonLog.h"
#include "Protocol.h"
#include "mock/mock_IODevice.h"

#include <iterator>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

using common::IODevice;
using common::mock_IODevice;

using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetArrayArgument;

namespace {
const uint8_t kCodebit_MCPU = 0x44;
const int32_t kCmdLength = 3;
const int32_t kExtLenLength = 3;
const int32_t kFrameNumberLength = 2;
const int32_t kTotalFramesLength = 2;
}  // namespace

class ProtocolTest : public ::testing::Test {
public:
    ProtocolTest()
    {
        m_regularMessage = {1, 2, 3, 'd', 'a', 't', 'a'};
        m_regularMessageFrame = {STX, 9, 1, 2, 3, 'd', 'a', 't', 'a', ETX};
        uint8_t cs =
            common::checksum(std::next(m_regularMessageFrame.begin()), m_regularMessageFrame.end());
        m_regularMessageFrame.push_back(cs);
        m_regularMessageProtocolLength = m_regularMessageFrame[1];
        m_regularMessageProtocolChecksum = cs;

        m_extendedLengthMessage = {3, 2, 1};
        m_extendedLengthMessage.insert(m_extendedLengthMessage.end(), 255, 'e');

        uint32_t frameLength = 263;
        m_extendedLengthMessageFrame = {STX, EXT_LEN};
        m_extendedLengthMessageFrame.push_back(3);
        m_extendedLengthMessageFrame.push_back(2);
        m_extendedLengthMessageFrame.push_back(1);
        m_extendedLengthMessageFrame.push_back(
            static_cast<uint8_t>((frameLength >> 16) & 0x000000ff));
        m_extendedLengthMessageFrame.push_back(
            static_cast<uint8_t>((frameLength >> 8) & 0x000000ff));
        m_extendedLengthMessageFrame.push_back(static_cast<uint8_t>(frameLength & 0x000000ff));

        m_extendedLengthMessageFrame.insert(m_extendedLengthMessageFrame.end(), 255, 'e');
        m_extendedLengthMessageFrame.push_back(ETX);
        cs = common::checksum(std::next(m_extendedLengthMessageFrame.begin()),
                              m_extendedLengthMessageFrame.end());
        m_extendedLengthMessageFrame.push_back(cs);

        const uint32_t kFrameDivisionMessageDataLength = 1200;
        const uint32_t kMaxFrameDataLength = 1024;
        const uint32_t kFrameDivisionMessageFirstFrameLength = kMaxFrameDataLength + 12;
        const uint32_t kFrameDivisionMessageSecondFrameLength =
            kFrameDivisionMessageDataLength - kMaxFrameDataLength + 12;

        m_frameDivisionMessage = {7, 8, kCodebit_MCPU | 0x02};
        m_frameDivisionMessage.insert(m_frameDivisionMessage.end(), kFrameDivisionMessageDataLength,
                                      'f');

        uint32_t numberOfFrames = (kFrameDivisionMessageDataLength / kMaxFrameDataLength) + 1;
        m_frameDivisionMessageFrame1 = {STX, EXT_LEN};
        m_frameDivisionMessageFrame1.push_back(7);
        m_frameDivisionMessageFrame1.push_back(8);
        m_frameDivisionMessageFrame1.push_back(kCodebit_MCPU | 0x02);

        m_frameDivisionMessageFrame1.push_back(
            static_cast<uint8_t>((kFrameDivisionMessageFirstFrameLength >> 16) & 0x000000ff));
        m_frameDivisionMessageFrame1.push_back(
            static_cast<uint8_t>((kFrameDivisionMessageFirstFrameLength >> 8) & 0x000000ff));
        m_frameDivisionMessageFrame1.push_back(
            static_cast<uint8_t>(kFrameDivisionMessageFirstFrameLength & 0x000000ff));
        uint32_t frameNumber = 0;
        m_frameDivisionMessageFrame1.push_back(
            static_cast<uint8_t>((frameNumber >> 8) & 0x000000ff));
        m_frameDivisionMessageFrame1.push_back(static_cast<uint8_t>(frameNumber & 0x000000ff));
        m_frameDivisionMessageFrame1.push_back(
            static_cast<uint8_t>((numberOfFrames >> 8) & 0x000000ff));
        m_frameDivisionMessageFrame1.push_back(static_cast<uint8_t>(numberOfFrames & 0x000000ff));
        m_frameDivisionMessageFrame1.insert(m_frameDivisionMessageFrame1.end(), kMaxFrameDataLength,
                                            'f');
        m_frameDivisionMessageFrame1.push_back(ETX);
        cs = common::checksum(std::next(m_frameDivisionMessageFrame1.begin()),
                              m_frameDivisionMessageFrame1.end());
        m_frameDivisionMessageFrame1.push_back(cs);

        m_frameDivisionMessageFrame2 = {STX, EXT_LEN};
        m_frameDivisionMessageFrame2.push_back(7);
        m_frameDivisionMessageFrame2.push_back(8);
        m_frameDivisionMessageFrame2.push_back(kCodebit_MCPU | 0x03);

        m_frameDivisionMessageFrame2.push_back(
            static_cast<uint8_t>((kFrameDivisionMessageSecondFrameLength >> 16) & 0x000000ff));
        m_frameDivisionMessageFrame2.push_back(
            static_cast<uint8_t>((kFrameDivisionMessageSecondFrameLength >> 8) & 0x000000ff));
        m_frameDivisionMessageFrame2.push_back(
            static_cast<uint8_t>(kFrameDivisionMessageSecondFrameLength & 0x000000ff));
        frameNumber = 1;
        m_frameDivisionMessageFrame2.push_back(
            static_cast<uint8_t>((frameNumber >> 8) & 0x000000ff));
        m_frameDivisionMessageFrame2.push_back(static_cast<uint8_t>(frameNumber & 0x000000ff));
        m_frameDivisionMessageFrame2.push_back(
            static_cast<uint8_t>((numberOfFrames >> 8) & 0x000000ff));
        m_frameDivisionMessageFrame2.push_back(static_cast<uint8_t>(numberOfFrames & 0x000000ff));

        m_frameDivisionMessageFrame2.insert(m_frameDivisionMessageFrame2.end(),
                                            kFrameDivisionMessageDataLength - kMaxFrameDataLength,
                                            'f');
        m_frameDivisionMessageFrame2.push_back(ETX);
        cs = common::checksum(std::next(m_frameDivisionMessageFrame2.begin()),
                              m_frameDivisionMessageFrame2.end());
        m_frameDivisionMessageFrame2.push_back(cs);
    }

    ~ProtocolTest() {}

    void SetUp() { daemon::InitializeCpuComLogMessages(); }

    void TearDown() { daemon::TerminateCpuComLogMessages(); }

public:
    // test message, regular frame, no extended length, no frame division
    std::vector<uint8_t> m_regularMessage;
    std::vector<uint8_t> m_regularMessageFrame;
    uint8_t m_regularMessageProtocolLength;
    uint8_t m_regularMessageProtocolChecksum;

    std::vector<uint8_t> m_extendedLengthMessage;
    std::vector<uint8_t> m_extendedLengthMessageFrame;

    std::vector<uint8_t> m_frameDivisionMessage;
    std::vector<uint8_t> m_frameDivisionMessageFrame1;
    std::vector<uint8_t> m_frameDivisionMessageFrame2;
};

TEST_F(ProtocolTest, ProtocolOpensAndClosesDevice)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
    EXPECT_CALL(*device, close()).Times(1);
    Protocol protocol(std::move(device));
}

// 6.3.1 Basic data link establishment method.
TEST_F(ProtocolTest, SendingFrame)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
    }

    Protocol protocol(std::move(device));
    protocol.send(m_regularMessage);
}

TEST_F(ProtocolTest, SendingFrameErrorResend)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Error, 1)));

        // Resending frame
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
    }

    Protocol protocol(std::move(device));
    protocol.send(m_regularMessage);
}

TEST_F(ProtocolTest, SendRetryAttemptsForPortReinit)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        constexpr int NUMBER_ATTEMPTS_BETWEEN_REOPEN_DEVICE = 6;

        EXPECT_CALL(*device, read(_, _))
            .Times(NUMBER_ATTEMPTS_BETWEEN_REOPEN_DEVICE)
            .WillRepeatedly(Return(IODevice::Result::Error));

        EXPECT_CALL(*device, close()).Times(1);
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);

        // Resending frame
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
    }

    Protocol protocol(std::move(device));
    protocol.send(m_regularMessage);
}

TEST_F(ProtocolTest, SendFrameRetryPermanentError)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());

    constexpr uint32_t MAX_COUNT_OF_SEND_ATTEMPT = 240;
    constexpr uint32_t NUMBER_ATTEMPTS_BETWEEN_REOPEN_DEVICE = 6;
    constexpr uint32_t REOPEN_DEVICE_COUNT =
        MAX_COUNT_OF_SEND_ATTEMPT / NUMBER_ATTEMPTS_BETWEEN_REOPEN_DEVICE;

    EXPECT_CALL(*device, read(_, _))
        .Times(MAX_COUNT_OF_SEND_ATTEMPT)
        .WillRepeatedly(Return(IODevice::Result::Error));
    EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(REOPEN_DEVICE_COUNT);
    EXPECT_CALL(*device, close()).Times(REOPEN_DEVICE_COUNT);

    Protocol protocol(std::move(device));
    ASSERT_EQ(protocol.send(m_regularMessage), false);
}

TEST_F(ProtocolTest, SendRetryEnquiryAfterError)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Error));

        // Resending frame
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Error, 0)));

        // Resending frame
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
    }

    Protocol protocol(std::move(device));
    protocol.send(m_regularMessage);
}

TEST_F(ProtocolTest, SendRetryAcknowledgementAfterError)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Error)));

        // Resending frame
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
    }

    Protocol protocol(std::move(device));
    protocol.send(m_regularMessage);
}

TEST_F(ProtocolTest, SendRetryNegativeAcknowledgementAfterError)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(NAK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Error, 0)));

        // Resending frame
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 0)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
    }

    Protocol protocol(std::move(device));
    protocol.send(m_regularMessage);
}

// 6.3.1 Basic data link establishment method.
TEST_F(ProtocolTest, ReceivingFrame)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, m_regularMessage.size() - kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(std::next(m_regularMessage.begin(), kCmdLength),
                                                m_regularMessage.end()),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolChecksum)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

TEST_F(ProtocolTest, ReceivingStartTextWithError)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Error)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

TEST_F(ProtocolTest, ReceivingPollingWithError)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Error));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

TEST_F(ProtocolTest, ReceivingEnquiryWithTimeoutOrError)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Timeout)));

        // Resending frame
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Error)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

TEST_F(ProtocolTest, ReceivingFrameWithMinLength)
{
    constexpr uint32_t MIN_FRAME_LENGTH = 5;

    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(MIN_FRAME_LENGTH - 1)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(NAK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Error, 0)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

TEST_F(ProtocolTest, ReceivingLengthWithTimeoutOrError)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Timeout)));
        EXPECT_CALL(*device, write(NAK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        // Resending frame
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Error)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

TEST_F(ProtocolTest, ReceivingCommandDataWithTimeoutOrError)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Timeout)));
        EXPECT_CALL(*device, write(NAK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        // Resending frame
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Error)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

TEST_F(ProtocolTest, ReceivingFrameAfterSeveralEnquiries)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(5)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, m_regularMessage.size() - kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(std::next(m_regularMessage.begin(), kCmdLength),
                                                m_regularMessage.end()),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolChecksum)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

// 6.3.3 Receiving other than ENQ in idle state
TEST_F(ProtocolTest, ReceivingOtherThanEnquiryInIdleState)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(NAK)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(5)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(0xff)),
                            Return(IODevice::Result::Success)))
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(0xfe)),
                            Return(IODevice::Result::Success)))
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(0xfd)),
                            Return(IODevice::Result::Success)))
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, m_regularMessage.size() - kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(std::next(m_regularMessage.begin(), kCmdLength),
                                                m_regularMessage.end()),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolChecksum)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
    }
    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);

    EXPECT_EQ(protocol.r1(), 2u);
    EXPECT_EQ(protocol.r2(), 0u);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

TEST_F(ProtocolTest, ReceivingRetryMaxNumberAttempts)
{
    constexpr uint32_t MAX_COUNT_OF_RECEIVE_ATTEMPT = 5;

    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        for (int i = 0; i < MAX_COUNT_OF_RECEIVE_ATTEMPT; i++) {
            EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
            EXPECT_CALL(*device, read(_, _))
                .Times(2)
                .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(NAK)),
                                Return(IODevice::Result::Success)))
                .WillOnce(Return(IODevice::Result::Timeout));

            // Resending frame
            EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
            EXPECT_CALL(*device, read(_, _))
                .Times(2)
                .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                                Return(IODevice::Result::Success)))
                .WillOnce(Return(IODevice::Result::Timeout));
        }
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);

    EXPECT_EQ(protocol.r1(), MAX_COUNT_OF_RECEIVE_ATTEMPT);
    EXPECT_EQ(protocol.r2(), 0u);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

// 6.3.4 ENQ reception, ENQ reception (ENQ collision)
TEST_F(ProtocolTest, ResolvingEnquiryCollision)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(NAK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 0)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
    }

    Protocol protocol(std::move(device));
    protocol.send(m_regularMessage);
}

// 6.3.5 Receive NAK for ENQ transmission (BUSY)
TEST_F(ProtocolTest, ReceivingNegativeAcknowledgementForTransmissionEnquiry)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(NAK)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 0)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
    }

    Protocol protocol(std::move(device));
    protocol.send(m_regularMessage);
}

// 6.3.6 Receiving ENQ, ACK, NAK other than ENK transmission (error)
TEST_F(ProtocolTest, ReceivingUnexpectedForTransmissionEnquiry)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(42)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 0)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
    }

    Protocol protocol(std::move(device));
    protocol.send(m_regularMessage);

    EXPECT_EQ(protocol.r1(), 0u);
    EXPECT_EQ(protocol.r2(), 2u);
}

// 6.3.7 No response to ENQ transmission (error)
TEST_F(ProtocolTest, HandlingNoResponseToTransmissionEnquiry)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 0)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
    }

    Protocol protocol(std::move(device));
    protocol.send(m_regularMessage);
}

// 6.3.8 Receiving other than STX for ACK transmission
TEST_F(ProtocolTest, ReceivingOtherThanStartTextAfterAcknowledgementTransmission)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(42)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(NAK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, m_regularMessage.size() - kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(std::next(m_regularMessage.begin(), kCmdLength),
                                                m_regularMessage.end()),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolChecksum)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

// 6.3.9 No response such as STX to ACK transmission
TEST_F(ProtocolTest, HandlingNoStartTextAfterAcknowledgementTransmission)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(NAK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, m_regularMessage.size() - kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(std::next(m_regularMessage.begin(), kCmdLength),
                                                m_regularMessage.end()),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolChecksum)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

// 6.3.11 Can not receive to CS after receiving STX, Len
TEST_F(ProtocolTest, ReceivingIncompleteFrameAfterStartTextAndLengthReceived)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, m_regularMessage.size() - kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(std::next(m_regularMessage.begin(), kCmdLength),
                                                m_regularMessage.end()),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(NAK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, m_regularMessage.size() - kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(std::next(m_regularMessage.begin(), kCmdLength),
                                                m_regularMessage.end()),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolChecksum)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

// 6.3.12 Receive NAK after STX ~ CS transmission (CheckSum error)
TEST_F(ProtocolTest, ProtocolDeniesFrameIfChecksumsDoNotMatch)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, m_regularMessage.size() - kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(std::next(m_regularMessage.begin(), kCmdLength),
                                                m_regularMessage.end()),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolChecksum + 1)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(NAK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, m_regularMessage.size() - kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(std::next(m_regularMessage.begin(), kCmdLength),
                                                m_regularMessage.end()),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolChecksum)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

TEST_F(ProtocolTest, ReceivingChecksumWithTimeoutOrError)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, m_regularMessage.size() - kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(std::next(m_regularMessage.begin(), kCmdLength),
                                                m_regularMessage.end()),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolChecksum + 1)),
                      Return(IODevice::Result::Timeout)));
        EXPECT_CALL(*device, write(NAK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        // Resending frame
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, m_regularMessage.size() - kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(std::next(m_regularMessage.begin(), kCmdLength),
                                                m_regularMessage.end()),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolChecksum)),
                      Return(IODevice::Result::Error)));
        EXPECT_CALL(*device, write(NAK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Error, 1)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

// 6.3.13 No response after STX ~ CS transmission
TEST_F(ProtocolTest, NotReceivingResponseAfterChecksumTransmission)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
    }

    Protocol protocol(std::move(device));
    protocol.send(m_regularMessage);
}

// 6.3.14 Continuous transmission of information messages
TEST_F(ProtocolTest, SendingSeveralFrames)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 0)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 0)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
    }

    Protocol protocol(std::move(device));
    protocol.send(m_regularMessage);
    protocol.send(m_regularMessage);
    protocol.send(m_regularMessage);
}

// 6.3.14 Continuous transmission of information messages
TEST_F(ProtocolTest, ReceivingSeveralFrames)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, m_regularMessage.size() - kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(std::next(m_regularMessage.begin(), kCmdLength),
                                                m_regularMessage.end()),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolChecksum)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, m_regularMessage.size() - kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(std::next(m_regularMessage.begin(), kCmdLength),
                                                m_regularMessage.end()),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolChecksum)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));

        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(m_regularMessage.begin(),
                                                std::next(m_regularMessage.begin(), kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, m_regularMessage.size() - kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(std::next(m_regularMessage.begin(), kCmdLength),
                                                m_regularMessage.end()),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArgPointee<0>(static_cast<uint8_t>(m_regularMessageProtocolChecksum)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_regularMessage.begin()), true);
}

TEST_F(ProtocolTest, SendingExtendedLengthFrame)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 0)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
    }

    Protocol protocol(std::move(device));
    protocol.send(m_extendedLengthMessage);
}

TEST_F(ProtocolTest, ReceivingExtendedLengthFrame)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(EXT_LEN)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(
                                std::next(m_extendedLengthMessageFrame.begin(), 2),
                                std::next(m_extendedLengthMessageFrame.begin(), 2 + kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kExtLenLength, _))
            .Times(1)
            .WillOnce(DoAll(
                SetArrayArgument<0>(std::next(m_extendedLengthMessageFrame.begin(), 2 + kCmdLength),
                                    std::next(m_extendedLengthMessageFrame.begin(),
                                              2 + kCmdLength + kExtLenLength)),
                Return(IODevice::Result::Success)));
        EXPECT_CALL(
            *device,
            readMulti(_, m_extendedLengthMessageFrame.size() - (2 + kCmdLength + kExtLenLength + 2),
                      _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(std::next(m_extendedLengthMessageFrame.begin(),
                                                          2 + kCmdLength + kExtLenLength),
                                                std::prev(m_extendedLengthMessageFrame.end(), 2)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(
                                *std::prev(m_extendedLengthMessageFrame.end()))),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_extendedLengthMessage.begin()), true);
}

TEST_F(ProtocolTest, ReceivingExtendedLengthFrameWithTimeoutOrError)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(EXT_LEN)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(
                                std::next(m_extendedLengthMessageFrame.begin(), 2),
                                std::next(m_extendedLengthMessageFrame.begin(), 2 + kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kExtLenLength, _))
            .Times(1)
            .WillOnce(DoAll(
                SetArrayArgument<0>(std::next(m_extendedLengthMessageFrame.begin(), 2 + kCmdLength),
                                    std::next(m_extendedLengthMessageFrame.begin(),
                                              2 + kCmdLength + kExtLenLength)),
                Return(IODevice::Result::Timeout)));
        EXPECT_CALL(*device, write(NAK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        // Resending frame
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(EXT_LEN)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(
                                std::next(m_extendedLengthMessageFrame.begin(), 2),
                                std::next(m_extendedLengthMessageFrame.begin(), 2 + kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kExtLenLength, _))
            .Times(1)
            .WillOnce(DoAll(
                SetArrayArgument<0>(std::next(m_extendedLengthMessageFrame.begin(), 2 + kCmdLength),
                                    std::next(m_extendedLengthMessageFrame.begin(),
                                              2 + kCmdLength + kExtLenLength)),
                Return(IODevice::Result::Error)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_extendedLengthMessage.begin()), true);
}

TEST_F(ProtocolTest, SendingFrameDivisionFrames)
{
    std::unique_ptr<mock_IODevice> device(new NiceMock<mock_IODevice>());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 0)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));

        EXPECT_CALL(*device, read(_, _)).Times(1).WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ENQ))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(_, _))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 0)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ACK)),
                            Return(IODevice::Result::Success)));
    }

    Protocol protocol(std::move(device));
    protocol.send(m_frameDivisionMessage);
}

TEST_F(ProtocolTest, ReceivingFrameDivisionFrames)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(EXT_LEN)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(
                                std::next(m_frameDivisionMessageFrame1.begin(), 2),
                                std::next(m_frameDivisionMessageFrame1.begin(), 2 + kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kExtLenLength, _))
            .Times(1)
            .WillOnce(DoAll(
                SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame1.begin(), 2 + kCmdLength),
                                    std::next(m_frameDivisionMessageFrame1.begin(),
                                              2 + kCmdLength + kExtLenLength)),
                Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kFrameNumberLength + kTotalFramesLength, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame1.begin(),
                                                    2 + kCmdLength + kExtLenLength),
                                          std::next(m_frameDivisionMessageFrame1.begin(),
                                                    2 + kCmdLength + kExtLenLength +
                                                        kFrameNumberLength + kTotalFramesLength)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_,
                                       m_frameDivisionMessageFrame1.size() -
                                           (2 + kCmdLength + kExtLenLength + kFrameNumberLength +
                                            kTotalFramesLength + 2),
                                       _))
            .Times(1)
            .WillOnce(
                DoAll(SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame1.begin(),
                                                    2 + kCmdLength + kExtLenLength +
                                                        kFrameNumberLength + kTotalFramesLength),
                                          std::prev(m_frameDivisionMessageFrame1.end(), 2)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(
                                *std::prev(m_frameDivisionMessageFrame1.end()))),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(EXT_LEN)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(
                                std::next(m_frameDivisionMessageFrame2.begin(), 2),
                                std::next(m_frameDivisionMessageFrame2.begin(), 2 + kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kExtLenLength, _))
            .Times(1)
            .WillOnce(DoAll(
                SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame2.begin(), 2 + kCmdLength),
                                    std::next(m_frameDivisionMessageFrame2.begin(),
                                              2 + kCmdLength + kExtLenLength)),
                Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kFrameNumberLength + kTotalFramesLength, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame2.begin(),
                                                    2 + kCmdLength + kExtLenLength),
                                          std::next(m_frameDivisionMessageFrame2.begin(),
                                                    2 + kCmdLength + kExtLenLength +
                                                        kFrameNumberLength + kTotalFramesLength)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_,
                                       m_frameDivisionMessageFrame2.size() -
                                           (2 + kCmdLength + kExtLenLength + kFrameNumberLength +
                                            kTotalFramesLength + 2),
                                       _))
            .Times(1)
            .WillOnce(
                DoAll(SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame2.begin(),
                                                    2 + kCmdLength + kExtLenLength +
                                                        kFrameNumberLength + kTotalFramesLength),
                                          std::prev(m_frameDivisionMessageFrame2.end(), 2)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ETX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(
                                *std::prev(m_frameDivisionMessageFrame2.end()))),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_frameDivisionMessage.begin()), true);
}

TEST_F(ProtocolTest, ReceivingDataFrameNumberWithTimeoutOrError)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(EXT_LEN)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(
                                std::next(m_frameDivisionMessageFrame1.begin(), 2),
                                std::next(m_frameDivisionMessageFrame1.begin(), 2 + kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kExtLenLength, _))
            .Times(1)
            .WillOnce(DoAll(
                SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame1.begin(), 2 + kCmdLength),
                                    std::next(m_frameDivisionMessageFrame1.begin(),
                                              2 + kCmdLength + kExtLenLength)),
                Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kFrameNumberLength + kTotalFramesLength, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame1.begin(),
                                                    2 + kCmdLength + kExtLenLength),
                                          std::next(m_frameDivisionMessageFrame1.begin(),
                                                    2 + kCmdLength + kExtLenLength +
                                                        kFrameNumberLength + kTotalFramesLength)),
                      Return(IODevice::Result::Timeout)));
        EXPECT_CALL(*device, write(NAK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        // Resending frame
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(EXT_LEN)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(
                                std::next(m_frameDivisionMessageFrame1.begin(), 2),
                                std::next(m_frameDivisionMessageFrame1.begin(), 2 + kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kExtLenLength, _))
            .Times(1)
            .WillOnce(DoAll(
                SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame1.begin(), 2 + kCmdLength),
                                    std::next(m_frameDivisionMessageFrame1.begin(),
                                              2 + kCmdLength + kExtLenLength)),
                Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kFrameNumberLength + kTotalFramesLength, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame1.begin(),
                                                    2 + kCmdLength + kExtLenLength),
                                          std::next(m_frameDivisionMessageFrame1.begin(),
                                                    2 + kCmdLength + kExtLenLength +
                                                        kFrameNumberLength + kTotalFramesLength)),
                      Return(IODevice::Result::Error)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_frameDivisionMessage.begin()), true);
}

TEST_F(ProtocolTest, ReceivingDataFrameWithTimeoutOrError)
{
    std::unique_ptr<mock_IODevice> device(new mock_IODevice());
    {
        InSequence sequence;
        EXPECT_CALL(*device, open(IODevice::OpenMode::ReadWrite)).Times(1);
        EXPECT_CALL(*device, close()).Times(1);
    }
    {
        InSequence sequence;
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(EXT_LEN)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(
                                std::next(m_frameDivisionMessageFrame1.begin(), 2),
                                std::next(m_frameDivisionMessageFrame1.begin(), 2 + kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kExtLenLength, _))
            .Times(1)
            .WillOnce(DoAll(
                SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame1.begin(), 2 + kCmdLength),
                                    std::next(m_frameDivisionMessageFrame1.begin(),
                                              2 + kCmdLength + kExtLenLength)),
                Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kFrameNumberLength + kTotalFramesLength, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame1.begin(),
                                                    2 + kCmdLength + kExtLenLength),
                                          std::next(m_frameDivisionMessageFrame1.begin(),
                                                    2 + kCmdLength + kExtLenLength +
                                                        kFrameNumberLength + kTotalFramesLength)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_,
                                       m_frameDivisionMessageFrame1.size() -
                                           (2 + kCmdLength + kExtLenLength + kFrameNumberLength +
                                            kTotalFramesLength + 2),
                                       _))
            .Times(1)
            .WillOnce(
                DoAll(SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame1.begin(),
                                                    2 + kCmdLength + kExtLenLength +
                                                        kFrameNumberLength + kTotalFramesLength),
                                          std::prev(m_frameDivisionMessageFrame1.end(), 2)),
                      Return(IODevice::Result::Timeout)));
        EXPECT_CALL(*device, write(NAK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));

        // Resending frame
        EXPECT_CALL(*device, poll(_)).Times(1).WillOnce(Return(IODevice::Result::Success));
        EXPECT_CALL(*device, read(_, _))
            .Times(2)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(ENQ)),
                            Return(IODevice::Result::Success)))
            .WillOnce(Return(IODevice::Result::Timeout));
        EXPECT_CALL(*device, write(ACK))
            .Times(1)
            .WillOnce(Return(std::make_pair(IODevice::Result::Success, 1)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(STX)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, read(_, _))
            .Times(1)
            .WillOnce(DoAll(SetArgPointee<0>(static_cast<uint8_t>(EXT_LEN)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kCmdLength, _))
            .Times(1)
            .WillOnce(DoAll(SetArrayArgument<0>(
                                std::next(m_frameDivisionMessageFrame1.begin(), 2),
                                std::next(m_frameDivisionMessageFrame1.begin(), 2 + kCmdLength)),
                            Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kExtLenLength, _))
            .Times(1)
            .WillOnce(DoAll(
                SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame1.begin(), 2 + kCmdLength),
                                    std::next(m_frameDivisionMessageFrame1.begin(),
                                              2 + kCmdLength + kExtLenLength)),
                Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_, kFrameNumberLength + kTotalFramesLength, _))
            .Times(1)
            .WillOnce(
                DoAll(SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame1.begin(),
                                                    2 + kCmdLength + kExtLenLength),
                                          std::next(m_frameDivisionMessageFrame1.begin(),
                                                    2 + kCmdLength + kExtLenLength +
                                                        kFrameNumberLength + kTotalFramesLength)),
                      Return(IODevice::Result::Success)));
        EXPECT_CALL(*device, readMulti(_,
                                       m_frameDivisionMessageFrame1.size() -
                                           (2 + kCmdLength + kExtLenLength + kFrameNumberLength +
                                            kTotalFramesLength + 2),
                                       _))
            .Times(1)
            .WillOnce(
                DoAll(SetArrayArgument<0>(std::next(m_frameDivisionMessageFrame1.begin(),
                                                    2 + kCmdLength + kExtLenLength +
                                                        kFrameNumberLength + kTotalFramesLength),
                                          std::prev(m_frameDivisionMessageFrame1.end(), 2)),
                      Return(IODevice::Result::Error)));
    }

    Protocol protocol(std::move(device));
    std::vector<uint8_t> data;
    protocol.receive(data);
    EXPECT_EQ(std::equal(data.begin(), data.end(), m_frameDivisionMessage.begin()), true);
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
