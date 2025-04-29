/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_SOCKET_MASTER_DEVICE_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_SOCKET_MASTER_DEVICE_H_

#include "IODevice.h"

#include <memory>
#include <string>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {
namespace socket {

class MasterDevice : public common::IODevice {
public:
    explicit MasterDevice(std::string deviceName, std::function<bool(int)> configure = {});
    virtual ~MasterDevice();
    MasterDevice(const MasterDevice& other) = delete;
    MasterDevice& operator=(const MasterDevice& other) = delete;
    MasterDevice(MasterDevice&& other);
    MasterDevice& operator=(MasterDevice&& other);

    friend void swap(MasterDevice& first, MasterDevice& second)
    {
        using std::swap;
        swap(static_cast<IODevice&>(first), static_cast<IODevice&>(second));
        swap(first.m_listend, second.m_listend);
    }

public:
    virtual bool open(OpenMode mode) override;
    virtual void close() override;

protected:
    explicit MasterDevice();

private:
    int m_listend;
};

}  // namespace socket
}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_SOCKET_MASTER_DEVICE_H_
