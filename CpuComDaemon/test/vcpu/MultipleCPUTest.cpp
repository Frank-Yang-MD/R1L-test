/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "MockProtocol.h"

#include "CPUCommon.h"
#include "MultipleCPU.h"
#include "Protocol.h"

#include <memory>

#include <gtest/gtest.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

using ::testing::_;
using ::testing::DoAll;
using ::testing::Mock;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SetArgReferee;
using ::testing::Test;

class MultipleCPUTest : public Test {
protected:
    MultipleCPUTest();
    ~MultipleCPUTest();

    uint8_t mAddress = kAddressMCPU;

    std::unique_ptr<MockProtocol> mProtocolReceive;
    std::unique_ptr<MockProtocol> mProtocolTransmit;

    MockProtocol* mProtocolReceiveRaw;
    MockProtocol* mProtocolTransmitRaw;

    MultipleCPU mMultipleCPU;
};

MultipleCPUTest::MultipleCPUTest()
    : mProtocolReceive{std::make_unique<MockProtocol>()}
    , mProtocolTransmit{std::make_unique<MockProtocol>()}
    , mProtocolReceiveRaw{mProtocolReceive.get()}
    , mProtocolTransmitRaw{mProtocolTransmit.get()}
    , mMultipleCPU{std::move(mProtocolReceive), std::move(mProtocolTransmit), mAddress}
{
}

MultipleCPUTest::~MultipleCPUTest() {}

TEST_F(MultipleCPUTest, initialize_mustReturnTrue) { ASSERT_TRUE(mMultipleCPU.initialize()); }

TEST_F(MultipleCPUTest, write_mustCallSendMethodOnProtocolTransmit)
{
    constexpr common::CpuCommand cpuCommand{0x01, 0x02};
    const std::vector<uint8_t> data{0xF1, 0xF2, 0xF3};

    auto arg = pack(cpuCommand, getSendCodebit(mAddress), data);

    EXPECT_CALL(*mProtocolTransmitRaw, send(arg)).Times(1).WillOnce(Return(true));

    mMultipleCPU.write(cpuCommand, data);
}

TEST_F(MultipleCPUTest, read_mustReturnFalseIfProtocolReceiveReturnFalse)
{
    std::pair<common::CpuCommand, std::vector<uint8_t>> outResult;

    EXPECT_CALL(*mProtocolReceiveRaw, receive(_)).WillOnce(Return(false));

    ASSERT_FALSE(mMultipleCPU.read(outResult));
}

TEST_F(MultipleCPUTest, read_mustReturnFalseIfCodeBitIsNotReceive)
{
    constexpr common::CpuCommand cpuCommand{0x01, 0x02};
    const std::vector<uint8_t> cpuCmdData{0xF1, 0xF2, 0xF3};
    auto arg = pack(cpuCommand, getSendCodebit(mAddress), cpuCmdData);

    std::pair<common::CpuCommand, std::vector<uint8_t>> outResult;

    EXPECT_CALL(*mProtocolReceiveRaw, receive(_))
        .WillOnce(DoAll(SetArgReferee<0>(arg), Return(true)));

    ASSERT_FALSE(mMultipleCPU.read(outResult));
}

TEST_F(MultipleCPUTest, read_mustReturnTrueIfCodeBitIsReceive)
{
    constexpr common::CpuCommand cpuCommand{0x01, 0x02};
    const std::vector<uint8_t> cpuCmdData{0xF1, 0xF2, 0xF3};
    auto arg = pack(cpuCommand, getReceiveCodebit(mAddress), cpuCmdData);

    std::pair<common::CpuCommand, std::vector<uint8_t>> outResult;

    EXPECT_CALL(*mProtocolReceiveRaw, receive(_))
        .WillOnce(DoAll(SetArgReferee<0>(arg), Return(true)));

    ASSERT_TRUE(mMultipleCPU.read(outResult));
}

TEST_F(MultipleCPUTest, read_mustReturnValidCpuCommandAndData)
{
    constexpr common::CpuCommand cpuCommand{0x01, 0x02};
    const std::vector<uint8_t> cpuCmdData{0xF1, 0xF2, 0xF3};
    auto arg = pack(cpuCommand, getReceiveCodebit(mAddress), cpuCmdData);

    std::pair<common::CpuCommand, std::vector<uint8_t>> outResult;

    EXPECT_CALL(*mProtocolReceiveRaw, receive(_))
        .WillOnce(DoAll(SetArgReferee<0>(arg), Return(true)));

    mMultipleCPU.read(outResult);

    const common::CpuCommand outCpuCmd = outResult.first;
    const std::vector<uint8_t>* outData = &outResult.second;

    ASSERT_EQ(outCpuCmd.first, cpuCommand.first);
    ASSERT_EQ(outCpuCmd.second, cpuCommand.second);
    ASSERT_EQ(*outData, cpuCmdData);
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
