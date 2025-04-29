/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCK_IUART_DEVICE_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCK_IUART_DEVICE_H_

#include "IUARTDevice.h"

#include <gmock/gmock.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class MockIUARTDevice : public IUARTDevice {
public:
    MOCK_METHOD2(getTerminalParameters, int(int, struct termios*));
    MOCK_METHOD3(setTerminalParameters, int(int, int, struct termios*));
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCK_IUART_DEVICE_H_
