/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IIODEVICE_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IIODEVICE_H_

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

/*
 * IoDevice - interface for operating with file descriptor.
 */
class IIoDevice {
public:
    // LCOV_EXCL_START
    virtual ~IIoDevice() = default;
    // LCOV_EXCL_STOP

    /*
     * Create a file descriptor for event notification
     */
    virtual int eventfd(unsigned int initValue, int flags) = 0;

    /*
     * Prepare file descriptors for polling
     */
    virtual void pollInit(int socketFd, int stopFd) = 0;

    /*
     * Wait for some event on a file descriptor
     */
    virtual int poll() = 0;

    /*
     * Check if socket fd receive POLLIN event
     */
    virtual bool isSocketFdReceivePollInEvent() = 0;

    /*
     * Check if socket fd receive POLLHUP event
     */
    virtual bool isSocketFdReceivePollHupEvent() = 0;

    /*
     * Check if stop fd receive POLLIN event
     */
    virtual bool isStopFdReceivePollInEvent() = 0;

    /*
     * Check if stop fd receive POLLHUP event
     */
    virtual bool isStopFdReceivePollHupEvent() = 0;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IIODEVICE_H_
