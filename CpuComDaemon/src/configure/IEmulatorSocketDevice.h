/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM__IEMULATORSOCKET_DEVICE_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM__IEMULATORSOCKET_DEVICE_H_

#include <fcntl.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

/**
 * IEmulatorSocketDevice is the interface to wrap system-dependent
 * socket functions.
 */
class IEmulatorSocketDevice {
public:
    // LCOV_EXCL_START
    virtual ~IEmulatorSocketDevice() = default;
    // LCOV_EXCL_STOP

    /**
     * Provides for control over open file.
     * @param fd - an open file descriptor.
     * @param command - command to be executed on the file
     * @param flags - additional flags
     */
    virtual int controlOpenFile(int fd, int command, int flags) = 0;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM__IEMULATORSOCKET_DEVICE_H_
