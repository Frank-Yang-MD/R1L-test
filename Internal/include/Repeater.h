/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef COM_MITSUBISHIELECTRIC_AHU_CPUCOM_REPEATER_H_
#define COM_MITSUBISHIELECTRIC_AHU_CPUCOM_REPEATER_H_

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

class Repeater {
public:
    explicit Repeater(std::function<void()> callable, std::chrono::milliseconds interval);
    ~Repeater();

public:
    void start();
    void stop();

private:
    void threadFunction();

private:
    std::function<void()> m_callable;
    std::chrono::milliseconds m_interval;
    std::atomic_bool m_stopFlag;
    std::unique_ptr<std::thread> m_thread;
};

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
#endif  // COM_MITSUBISHIELECTRIC_AHU_CPUCOM_REPEATER_H_
