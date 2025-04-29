/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "SlaveDevice.h"
#include <cutils/sockets.h>

#include <chrono>
#include <thread>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {
namespace socket {

bool SlaveDevice::open(OpenMode /*mode*/)
{
    bool result = false;
    m_fd =
        socket_local_client(m_deviceName.c_str(), ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    while (m_fd < 0) {
        m_fd = socket_local_client(m_deviceName.c_str(), ANDROID_SOCKET_NAMESPACE_ABSTRACT,
                                   SOCK_STREAM);

        // This is an UGLY workaround to the undefined emulator/cpucomdaemon start order.
        // This class is not used in production code, only in an emulator environment.
        // This workaround here alows us to hide emulator from production code.
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(50ms);
    }
    if (m_fd >= 0) {
        result = m_configure ? m_configure(m_fd) : true;
    }
    return result;
}

}  // namespace socket
}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com