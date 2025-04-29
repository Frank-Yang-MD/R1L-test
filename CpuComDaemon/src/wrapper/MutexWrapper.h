/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MUTEXWRAPPER_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MUTEXWRAPPER_H_

#include "IMutexWrapper.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {

class MutexWrapper final : public IMutexWrapper {  // LCOV_EXCL_LINE
    // exclude all possible destructors, that cannot be covered by unit-test
public:
    void lock(std::mutex& mutex) override;

    void unlock(std::mutex& mutex) override;
};

}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_MUTEXWRAPPER_H_
