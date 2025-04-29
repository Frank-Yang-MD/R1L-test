/*
 * COPYRIGHT (C) 2022 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */
// LCOV_EXCL_START
// This is excluded from a unit test coverage report because this class has standard
// library code that cannot be covered by unit-test

#include "MutexWrapper.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {

void MutexWrapper::lock(std::mutex& mutex) { mutex.lock(); }

void MutexWrapper::unlock(std::mutex& mutex) { mutex.unlock(); }

}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com

// LCOV_EXCL_STOP