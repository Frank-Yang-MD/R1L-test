/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "CPU.h"
#include "CPUCommon.h"
#include "Protocol.h"

#include "MockProtocol.h"

#include <gtest/gtest.h>
#include <memory>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

using ::testing::_;
using ::testing::DoAll;
using ::testing::Mock;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::Test;

class CPUTest : public Test {
protected:
    CPUTest();
    ~CPUTest();

    uint8_t mAddress = kAddressMCPU;

    std::unique_ptr<MockProtocol> mProtocol;
    MockProtocol* mProtocolRaw = nullptr;

    CPU mCPU;
};

CPUTest::CPUTest()
    : mProtocol{std::make_unique<MockProtocol>()}
    , mProtocolRaw{mProtocol.get()}
    , mCPU{std::move(mProtocol), mAddress}
{
}

CPUTest::~CPUTest() {}

TEST_F(CPUTest, initialize_mustReturnTrue) { ASSERT_TRUE(mCPU.initialize()); }

TEST_F(CPUTest, write_mustCallSendMethodOnProtocol)
{
    constexpr common::CpuCommand cpuCommand{0x01, 0x02};
    const std::vector<uint8_t> data{0xF1, 0xF2, 0xF3};

    auto argument = pack(cpuCommand, getSendCodebit(mAddress), data);

    EXPECT_CALL(*mProtocolRaw, send(argument)).Times(1).WillOnce(Return(true));

    mCPU.write(cpuCommand, data);
}

TEST_F(CPUTest, read_mustReturnFalseIfProtocolReceiveReturnFalse)
{
    std::pair<common::CpuCommand, std::vector<uint8_t>> outResult;

    EXPECT_CALL(*mProtocolRaw, receive(_)).WillOnce(Return(false));

    ASSERT_FALSE(mCPU.read(outResult));
}

TEST_F(CPUTest, read_mustReturnFalseIfCodeBitIsNotReceive)
{
    constexpr common::CpuCommand cpuCommand{0x01, 0x02};
    const std::vector<uint8_t> cpuCmdData{0xF1, 0xF2, 0xF3};
    auto argument = pack(cpuCommand, getSendCodebit(mAddress), cpuCmdData);

    std::pair<common::CpuCommand, std::vector<uint8_t>> outResult;

    EXPECT_CALL(*mProtocolRaw, receive(_))
        .WillOnce(DoAll(SetArgReferee<0>(argument), Return(true)));

    ASSERT_FALSE(mCPU.read(outResult));
}

TEST_F(CPUTest, read_mustReturnTrueIfCodeBitIsReceive)
{
    constexpr common::CpuCommand cpuCommand{0x01, 0x02};
    const std::vector<uint8_t> cpuCmdData{0xF1, 0xF2, 0xF3};
    auto argument = pack(cpuCommand, getReceiveCodebit(mAddress), cpuCmdData);

    std::pair<common::CpuCommand, std::vector<uint8_t>> outResult;

    EXPECT_CALL(*mProtocolRaw, receive(_))
        .WillOnce(DoAll(SetArgReferee<0>(argument), Return(true)));

    ASSERT_TRUE(mCPU.read(outResult));
}

TEST_F(CPUTest, read_mustReturnValidCpuCommandAndData)
{
    constexpr common::CpuCommand cpuCommand{0x01, 0x02};
    const std::vector<uint8_t> cpuCmdData{0xF1, 0xF2, 0xF3};
    auto argument = pack(cpuCommand, getReceiveCodebit(mAddress), cpuCmdData);

    std::pair<common::CpuCommand, std::vector<uint8_t>> outResult;

    EXPECT_CALL(*mProtocolRaw, receive(_))
        .WillOnce(DoAll(SetArgReferee<0>(argument), Return(true)));

    mCPU.read(outResult);

    const common::CpuCommand outCpuCmd = outResult.first;
    const std::vector<uint8_t>& outData = outResult.second;

    ASSERT_EQ(outCpuCmd.first, cpuCommand.first);
    ASSERT_EQ(outCpuCmd.second, cpuCommand.second);
    ASSERT_EQ(outData, cpuCmdData);
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
