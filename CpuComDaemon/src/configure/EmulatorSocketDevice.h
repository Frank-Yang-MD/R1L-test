/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_EMULATORSOCKET_DEVICE_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_EMULATORSOCKET_DEVICE_H_

#include "IEmulatorSocketDevice.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

/**
 * This class implements interface to wrap system-dependent
 * socket functions.
 */
class EmulatorSocketDevice final : public IEmulatorSocketDevice {  // LCOV_EXCL_LINE
    // exclude all possible destructors, that cannot be covered by unit-test
public:
    int controlOpenFile(int fd, int command, int flags) override;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_EMULATORSOCKET_DEVICE_H_
