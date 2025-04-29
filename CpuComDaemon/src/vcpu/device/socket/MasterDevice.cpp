/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "MasterDevice.h"
#include <cutils/sockets.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {
namespace socket {

namespace {
const int kInvalidFileDescriptor = -1;
}

MasterDevice::MasterDevice(std::string deviceName, std::function<bool(int)> configure /*= {}*/)
    : IODevice(std::move(deviceName), configure)
    , m_listend(kInvalidFileDescriptor)
{
}

MasterDevice::MasterDevice()
    : IODevice()
    , m_listend(kInvalidFileDescriptor)
{
}

MasterDevice::~MasterDevice() { close(); }

MasterDevice::MasterDevice(MasterDevice&& other)
    : MasterDevice()
{
    swap(*this, other);
}

MasterDevice& MasterDevice::operator=(MasterDevice&& other)
{
    swap(*this, other);
    return *this;
}

bool MasterDevice::open(OpenMode /*mode*/)
{
    bool result = false;
    m_listend =
        socket_local_server(m_deviceName.c_str(), ANDROID_SOCKET_NAMESPACE_ABSTRACT, SOCK_STREAM);
    if (m_listend >= 0) {
        struct sockaddr_un client_addr;
        socklen_t client_addr_length = sizeof(client_addr);
        m_fd = accept(m_listend, (struct sockaddr*)&client_addr, &client_addr_length);
    }
    if (m_fd >= 0) {
        result = m_configure ? m_configure(m_fd) : true;
    }
    return result;
}

void MasterDevice::close()
{
    if (m_listend != kInvalidFileDescriptor) {
        ::close(m_listend);
        m_listend = kInvalidFileDescriptor;
    }
    IODevice::close();
}

}  // namespace socket
}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com