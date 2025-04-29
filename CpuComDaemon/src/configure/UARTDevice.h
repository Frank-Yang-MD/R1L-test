/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_UARTDEVICE_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_UARTDEVICE_H_

#include "IUARTDevice.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

/**
 * This class implements interface to wrap system-dependent
 * uart functions.
 */
class UARTDevice final : public IUARTDevice {  // LCOV_EXCL_LINE
    // exclude all possible destructors, that cannot be covered by unit-test
public:
    int getTerminalParameters(int fd, struct termios* data) override;

    int setTerminalParameters(int fd, int actions, struct termios* data) override;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_UARTDEVICE_H_
