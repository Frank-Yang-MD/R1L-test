/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "DeviceConfigurations.h"
#include "MockIEmulatorSocketDevice.h"

#include <gtest/gtest.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

using ::testing::An;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::Test;

class DeviceConfigureEmulatorSocketTest : public Test {
protected:
    DeviceConfigureEmulatorSocketTest();
    ~DeviceConfigureEmulatorSocketTest();

    NiceMock<MockIEmulatorSocketDevice> mDevice;

    DeviceConfigureEmulatorSocket mDeviceConfigureEmulatorSocket;
};

DeviceConfigureEmulatorSocketTest::DeviceConfigureEmulatorSocketTest()
    : mDeviceConfigureEmulatorSocket{mDevice}
{
}

DeviceConfigureEmulatorSocketTest::~DeviceConfigureEmulatorSocketTest() {}

TEST_F(DeviceConfigureEmulatorSocketTest,
       operatorInvoke_mustReturnFalseAfterGetFileStatusFlagWithError)
{
    const int fd = 1;

    EXPECT_CALL(mDevice, controlOpenFile(fd, F_GETFL, 0)).Times(1).WillOnce(Return(-1));
    EXPECT_CALL(mDevice, controlOpenFile(fd, F_SETFL, -1)).Times(0);

    bool result = mDeviceConfigureEmulatorSocket(fd);

    EXPECT_EQ(result, false);
}

TEST_F(DeviceConfigureEmulatorSocketTest, operatorInvoke_mustReturnTrueAfterSetFileStatusFlag)
{
    const int fd = 1;
    Sequence sequence;

    EXPECT_CALL(mDevice, controlOpenFile(fd, F_GETFL, An<int>()))
        .Times(1)
        .InSequence(sequence)
        .WillOnce(Return(1));
    EXPECT_CALL(mDevice, controlOpenFile(fd, F_SETFL, An<int>()))
        .Times(1)
        .InSequence(sequence)
        .WillOnce(Return(0));

    bool result = mDeviceConfigureEmulatorSocket(fd);

    EXPECT_EQ(result, true);
}

TEST_F(DeviceConfigureEmulatorSocketTest,
       operatorInvoke_mustReturnFalseAfterSetFileStatusFlagWithError)
{
    const int fd = 1;
    Sequence sequence;

    EXPECT_CALL(mDevice, controlOpenFile(fd, F_GETFL, An<int>()))
        .Times(1)
        .InSequence(sequence)
        .WillOnce(Return(0));
    EXPECT_CALL(mDevice, controlOpenFile(fd, F_SETFL, An<int>()))
        .Times(1)
        .InSequence(sequence)
        .WillOnce(Return(-1));

    bool result = mDeviceConfigureEmulatorSocket(fd);

    EXPECT_EQ(result, false);
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
