/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */
// LCOV_EXCL_START
// This is excluded from a unit test coverage report because this class has system-dependency code
// that cannot be covered by unit-test

#include "EmulatorSocketDevice.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

int EmulatorSocketDevice::controlOpenFile(int fd, int command, int flags)
{
    int result = ::fcntl(fd, command, flags);
    return result;
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

// LCOV_EXCL_STOP
