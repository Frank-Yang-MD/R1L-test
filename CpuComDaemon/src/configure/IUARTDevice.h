/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IUART_DEVICE_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IUART_DEVICE_H_

#include <termios.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

/**
 * IUARTDevice is the interface to wrap system-dependent
 * uart functions.
 */
class IUARTDevice {
public:
    // LCOV_EXCL_START
    virtual ~IUARTDevice() = default;
    // LCOV_EXCL_STOP

    /**
     * Get the parameters associated with the terminal.
     * @param fd - an open file descriptor associated with a terminal.
     * @param data - a pointer to a termios structure
     */
    virtual int getTerminalParameters(int fd, struct termios* data) = 0;

    /**
     * Set the parameters associated with the terminal
     * @param fd - an open file descriptor associated with a terminal.
     * @param actions - an optional actions
     * @param data - a pointer to a termios structure
     */
    virtual int setTerminalParameters(int fd, int actions, struct termios* data) = 0;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IUART_DEVICE_H_
