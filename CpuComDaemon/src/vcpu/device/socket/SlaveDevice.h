/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_SOCKET_SLAVE_DEVICE_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_SOCKET_SLAVE_DEVICE_H_

#include "IODevice.h"

#include <memory>
#include <string>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {
namespace socket {

class SlaveDevice : public common::IODevice {
public:
    using IODevice::IODevice;
    virtual bool open(OpenMode mode) override;
};

}  // namespace socket
}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_SOCKET_SLAVE_DEVICE_H_
