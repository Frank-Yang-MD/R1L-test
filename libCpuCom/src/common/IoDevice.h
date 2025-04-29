/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IODEVICE_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IODEVICE_H_

#include "IIoDevice.h"

#include <poll.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class IoDevice final : public IIoDevice {  // LCOV_EXCL_LINE
    // exclude all possible destructors, that cannot be covered by unit-test
public:
    int eventfd(unsigned int initValue, int flags) override;

    void pollInit(int socketFd, int stopFd) override;

    int poll() override;

    bool isSocketFdReceivePollInEvent() override;

    bool isSocketFdReceivePollHupEvent() override;

    bool isStopFdReceivePollInEvent() override;

    bool isStopFdReceivePollHupEvent() override;

private:
    constexpr static int k_numberOfDescriptors = 2;
    constexpr static int k_infiniteTimeout = -1;

    struct pollfd m_fds[k_numberOfDescriptors];
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IODEVICE_H_
