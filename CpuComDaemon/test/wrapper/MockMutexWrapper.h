/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MUTEXWRAPPERMOCK_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MUTEXWRAPPERMOCK_H_

#include "IMutexWrapper.h"

#include <gmock/gmock.h>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {

class MockMutexWrapper : public IMutexWrapper {
public:
    MOCK_METHOD1(lock, void(std::mutex&));
    MOCK_METHOD1(unlock, void(std::mutex&));
};

}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MUTEXWRAPPERMOCK_H_
