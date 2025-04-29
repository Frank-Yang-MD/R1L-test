/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */
// LCOV_EXCL_START
// This is excluded from a unit test coverage report because this class has standard
// library code that cannot be covered by unit-test

#include "IoDevice.h"

#include <sys/eventfd.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

int IoDevice::eventfd(unsigned int initValue, int flags) { return ::eventfd(initValue, flags); }

void IoDevice::pollInit(int socketFd, int stopFd)
{
    m_fds[0].fd = socketFd;
    m_fds[0].events = POLLIN;

    m_fds[1].fd = stopFd;
    m_fds[1].events = POLLIN;
}

int IoDevice::poll() { return ::poll(m_fds, k_numberOfDescriptors, k_infiniteTimeout); }

bool IoDevice::isSocketFdReceivePollInEvent() { return m_fds[0].revents & POLLIN; }

bool IoDevice::isSocketFdReceivePollHupEvent() { return m_fds[0].revents & POLLHUP; }

bool IoDevice::isStopFdReceivePollInEvent() { return m_fds[1].revents & POLLIN; }

bool IoDevice::isStopFdReceivePollHupEvent() { return m_fds[1].revents & POLLHUP; }

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

// LCOV_EXCL_STOP
