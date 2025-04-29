/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_DEVICE_CONFIGURATIONS_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_DEVICE_CONFIGURATIONS_H_

#include "IEmulatorSocketDevice.h"
#include "IUARTDevice.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {

class DeviceConfigureUART {
public:
    explicit DeviceConfigureUART(impl::IUARTDevice& device);
    bool operator()(int fd);

private:
    impl::IUARTDevice& mDevice;
};

class DeviceConfigureEmulatorSocket {
public:
    explicit DeviceConfigureEmulatorSocket(impl::IEmulatorSocketDevice& device);
    bool operator()(int fd);

private:
    impl::IEmulatorSocketDevice& mDevice;
};

}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_DEVICE_CONFIGURATIONS_H_
