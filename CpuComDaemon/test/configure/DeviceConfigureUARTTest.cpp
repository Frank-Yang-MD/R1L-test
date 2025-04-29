/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "DeviceConfigurations.h"
#include "MockIUARTDevice.h"

#include <gtest/gtest.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

using ::testing::An;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Test;

class DeviceConfigureUARTTest : public Test {
protected:
    DeviceConfigureUARTTest();
    ~DeviceConfigureUARTTest();

    NiceMock<MockIUARTDevice> mDevice;

    DeviceConfigureUART mDeviceConfigureUART;
};

DeviceConfigureUARTTest::DeviceConfigureUARTTest()
    : mDeviceConfigureUART{mDevice}
{
}

DeviceConfigureUARTTest::~DeviceConfigureUARTTest() {}

TEST_F(DeviceConfigureUARTTest, operatorInvoke_mustReturnFalseAfterGetTerminalParametersWithError)
{
    const int fd = 1;

    EXPECT_CALL(mDevice, getTerminalParameters(fd, An<struct termios*>()))
        .Times(1)
        .WillOnce(Return(-1));
    EXPECT_CALL(mDevice, setTerminalParameters(fd, TCSANOW, An<struct termios*>())).Times(0);

    bool result = mDeviceConfigureUART(fd);

    EXPECT_EQ(result, false);
}

TEST_F(DeviceConfigureUARTTest, operatorInvoke_mustReturnTrueAfterGetTerminalParameters)
{
    const int fd = 1;

    EXPECT_CALL(mDevice, getTerminalParameters(fd, An<struct termios*>()))
        .Times(1)
        .WillOnce(Return(0));
    EXPECT_CALL(mDevice, setTerminalParameters(fd, TCSANOW, An<struct termios*>()))
        .Times(1)
        .WillOnce(Return(0));

    bool result = mDeviceConfigureUART(fd);

    EXPECT_EQ(result, true);
}

TEST_F(DeviceConfigureUARTTest, operatorInvoke_mustReturnFalseAfterSetTerminalParametersWithError)
{
    const int fd = 1;

    EXPECT_CALL(mDevice, getTerminalParameters(fd, An<struct termios*>()))
        .Times(1)
        .WillOnce(Return(0));
    EXPECT_CALL(mDevice, setTerminalParameters(fd, TCSANOW, An<struct termios*>()))
        .Times(1)
        .WillOnce(Return(-1));

    bool result = mDeviceConfigureUART(fd);

    EXPECT_EQ(result, false);
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
