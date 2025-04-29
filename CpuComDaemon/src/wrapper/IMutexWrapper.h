/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IMUTEXWRAPPER_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IMUTEXWRAPPER_H_

#include <mutex>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {

/**
 * @brief The class interface for wrapping of mutexes.
 */
class IMutexWrapper {  // LCOV_EXCL_LINE
    // exclude all possible destructors, that cannot be covered by unit-test
public:
    // LCOV_EXCL_START
    virtual ~IMutexWrapper() = default;
    // LCOV_EXCL_STOP

    /**
     * @brief Locks mutex
     * @param mutex mutex for locking
     */
    virtual void lock(std::mutex& mutex) = 0;

    /**
     * @brief Unlocks mutex
     * @param mutex mutex for unlocking
     */
    virtual void unlock(std::mutex& mutex) = 0;
};

}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_IMUTEXWRAPPER_H_
