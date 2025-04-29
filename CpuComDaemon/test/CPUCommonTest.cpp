/*
 * COPYRIGHT (C) 2020 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "CPUCommon.h"

#include <gtest/gtest.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

using ::testing::Test;

using common::CpuCommand;

class CPUCommonTest : public Test {
protected:
    ~CPUCommonTest();

    const CpuCommand mCpuCommand{0x09, 0x08};
    const uint8_t mCodeBit{0x01};
    const std::vector<uint8_t> mData{0x02, 0x03, 0x0A, 0xFF};
};

CPUCommonTest::~CPUCommonTest() {}

TEST_F(CPUCommonTest, pack_mustReturnCorrectValue)
{
    const std::vector<uint8_t> expectedResult{0x09, 0x08, 0x01, 0x02, 0x03, 0x0A, 0xFF};

    ASSERT_EQ(pack(mCpuCommand, mCodeBit, mData), expectedResult);
}

TEST_F(CPUCommonTest, unpack_mustReturnCorrectValue)
{
    const std::vector<uint8_t> givenData{0x09, 0x08, 0x01, 0x02, 0x03, 0x0A, 0xFF};
    auto unpackResult = unpack(givenData);

    ASSERT_EQ(mCpuCommand, std::get<0>(unpackResult));
    ASSERT_EQ(mCodeBit, std::get<1>(unpackResult));
    ASSERT_EQ(mData, std::get<2>(unpackResult));
}

TEST_F(CPUCommonTest, getSendCodebit_withMCPUAddress)
{
    constexpr uint8_t address = kAddressMCPU;

    ASSERT_EQ(getSendCodebit(address), (kAddressVCPU << 5) + (kAddressMCPU << 2));
}

TEST_F(CPUCommonTest, getSendCodebit_withVCPUAddress)
{
    constexpr uint8_t address = kAddressVCPU;

    ASSERT_EQ(getSendCodebit(address), (kAddressMCPU << 5) + (kAddressVCPU << 2));
}

TEST_F(CPUCommonTest, getReceiveCodebit_withMCPUAddress)
{
    constexpr uint8_t address = kAddressMCPU;

    ASSERT_EQ(getReceiveCodebit(address), (kAddressMCPU << 5) + (kAddressVCPU << 2));
}

TEST_F(CPUCommonTest, getReceiveCodebit_withVCPUAddress)
{
    constexpr uint8_t address = kAddressVCPU;

    ASSERT_EQ(getReceiveCodebit(address), (kAddressVCPU << 5) + (kAddressMCPU << 2));
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
