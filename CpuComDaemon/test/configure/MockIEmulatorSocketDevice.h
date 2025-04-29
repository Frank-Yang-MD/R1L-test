/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCK_IEMULATORSOCKET_DEVICE_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCK_IEMULATORSOCKET_DEVICE_H_

#include "IEmulatorSocketDevice.h"

#include <gmock/gmock.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class MockIEmulatorSocketDevice : public IEmulatorSocketDevice {
public:
    MOCK_METHOD3(controlOpenFile, int(int, int, int));
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCK_IEMULATORSOCKET_DEVICE_H_
