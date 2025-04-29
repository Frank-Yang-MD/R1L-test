/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "DeviceConfigurations.h"

#include <unistd.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {

DeviceConfigureUART::DeviceConfigureUART(impl::IUARTDevice& device)
    : mDevice(device)
{
}

DeviceConfigureEmulatorSocket::DeviceConfigureEmulatorSocket(impl::IEmulatorSocketDevice& device)
    : mDevice(device)
{
}

bool DeviceConfigureUART::operator()(int fd)
{
    bool result = false;
    struct termios tio = {};
    int r = mDevice.getTerminalParameters(fd, &tio);
    if (r == -1) {
        result = false;
    }
    else {
        tio.c_iflag = 0x0000;
        tio.c_oflag = 0x0000;
        tio.c_cflag = B115200 | CS8 | CREAD | PARENB | HUPCL | CLOCAL;
        tio.c_lflag = ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN;
        tio.c_cflag &= ~CBAUD;
        tio.c_cflag |= B1000000;
        result = mDevice.setTerminalParameters(fd, TCSANOW, &tio) != -1;
    }
    return result;
}

bool DeviceConfigureEmulatorSocket::operator()(int fd)
{
    bool result = false;
    int flags = mDevice.controlOpenFile(fd, F_GETFL, 0);
    result = (flags != -1);
    if (result) {
        flags = flags | O_RDWR;
        flags = flags | O_NONBLOCK;
        flags = flags | O_NOCTTY;
        result = (mDevice.controlOpenFile(fd, F_SETFL, flags) == 0);
    }
    return result;
}

}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
