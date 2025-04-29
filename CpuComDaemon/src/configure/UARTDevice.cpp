/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */
// LCOV_EXCL_START
// This is excluded from a unit test coverage report because this class has system-dependency code
// that cannot be covered by unit-test

#include "UARTDevice.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

int UARTDevice::getTerminalParameters(int fd, struct termios* data)
{
    int result = ::tcgetattr(fd, data);
    return result;
}

int UARTDevice::setTerminalParameters(int fd, int actions, struct termios* data)
{
    int result = ::tcsetattr(fd, actions, data);
    return result;
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

// LCOV_EXCL_STOP
