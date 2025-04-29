/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "Repeater.h"

namespace com {
namespace mitsubishielectric {
namespace ahu {
namespace cpucom {
namespace impl {

Repeater::Repeater(std::function<void()> callable, std::chrono::milliseconds interval)
    : m_callable(callable)
    , m_interval(interval)
    , m_stopFlag(false)
    , m_thread(nullptr)
{
}

Repeater::~Repeater() { stop(); }

void Repeater::start()
{
    m_thread.reset(new std::thread(std::bind(&Repeater::threadFunction, this)));
}

void Repeater::stop()
{
    m_stopFlag = true;
    if (m_thread && m_thread->joinable()) {
        m_thread->join();
    }
}

void Repeater::threadFunction()
{
    while (m_stopFlag == false) {
        m_callable();
        std::this_thread::sleep_for(m_interval);
    }
}

}  // namespace impl
}  // namespace cpucom
}  // namespace ahu
}  // namespace mitsubishielectric
}  // namespace com
