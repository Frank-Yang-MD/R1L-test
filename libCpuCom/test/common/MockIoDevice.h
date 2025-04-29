/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCKIODEVICE_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCKIODEVICE_H_

#include "common/IIoDevice.h"

#include <gmock/gmock.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class MockIoDevice : public IIoDevice {
public:
    MOCK_METHOD2(eventfd, int(unsigned int, int));

    MOCK_METHOD2(pollInit, void(int, int));

    MOCK_METHOD0(poll, int());

    MOCK_METHOD0(isSocketFdReceivePollInEvent, bool());

    MOCK_METHOD0(isSocketFdReceivePollHupEvent, bool());

    MOCK_METHOD0(isStopFdReceivePollInEvent, bool());

    MOCK_METHOD0(isStopFdReceivePollHupEvent, bool());
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MOCKIODEVICE_H_
