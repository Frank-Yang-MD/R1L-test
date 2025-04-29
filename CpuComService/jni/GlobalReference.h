/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef com_mitsubishielectric_ahu_efw_cpucomservice_GlobalReference_h_
#define com_mitsubishielectric_ahu_efw_cpucomservice_GlobalReference_h_

#include <atomic>
#include <functional>
#include <jni.h>

namespace impl {
class ReferenceCounter {
public:
    explicit ReferenceCounter()
        : m_refs(0)
    {
    }

    ~ReferenceCounter() {}

public:
    void AddReference() { ++m_refs; }
    uint32_t ReleaseReference() { return --m_refs; }

private:
    std::atomic<uint32_t> m_refs;
};

inline JNIEnv* GetEnv(JavaVM* vm)
{
    // Calling thread must be attached to the Java VM.
    // WARNING: Don't call this function from the destructor of global object since at that point
    //          the calling (main) thread is already detached from Java VM.
    JNIEnv* env = nullptr;
    auto vm_state = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    (void)vm_state;
    return env;
}
}  // namespace impl

template <typename T>
class GlobalReference {
public:
    explicit GlobalReference()
        : m_vm(nullptr)
        , m_deleter([](JavaVM*, T) {})
        , m_value()
        , m_counter(nullptr)
    {
    }

    GlobalReference(T localRef, JavaVM* vm, std::function<void(JavaVM*, T)> deleter)
        : m_vm(vm)
        , m_deleter(deleter)
    {
        m_value = static_cast<T>(impl::GetEnv(m_vm)->NewGlobalRef(localRef));
        m_counter = new impl::ReferenceCounter();
        m_counter->AddReference();
    }

    GlobalReference(const GlobalReference<T>& other)
        : m_vm(other.m_vm)
        , m_deleter(other.m_deleter)
        , m_value(other.m_value)
        , m_counter(other.m_counter)
    {
        if (m_counter) {
            m_counter->AddReference();
        }
    }

    GlobalReference(GlobalReference<T>&& other)
        : m_vm(std::move(other.m_vm))
        , m_deleter(std::move(other.m_deleter))
        , m_value(std::move(other.m_value))
        , m_counter(std::move(other.m_counter))
    {
        other.m_vm = nullptr;
        other.m_deleter = [](JavaVM*, T) {};
        other.m_counter = nullptr;
    }

    GlobalReference<T>& operator=(const GlobalReference<T>& other)
    {
        if (this != &other) {
            if (m_counter && m_counter->ReleaseReference() == 0) {
                m_deleter(m_vm, m_value);
                delete m_counter;
            }
            m_vm = other.m_vm;
            m_deleter = other.m_deleter;
            m_value = other.m_value;
            m_counter = other.m_counter;
            if (m_counter) {
                m_counter->AddReference();
            }
        }
        return *this;
    }

    GlobalReference<T>& operator=(GlobalReference<T>&& other)
    {
        if (m_counter && m_counter->ReleaseReference() == 0) {
            m_deleter(m_vm, m_value);
            delete m_counter;
        }
        m_vm = std::move(other.m_vm);
        m_deleter = std::move(other.m_deleter);
        m_value = std::move(other.m_value);
        m_counter = std::move(other.m_counter);

        other.m_vm = nullptr;
        other.m_deleter = [](JavaVM*, T) {};
        other.m_counter = nullptr;
        return *this;
    }

    ~GlobalReference()
    {
        if (m_counter && m_counter->ReleaseReference() == 0) {
            m_deleter(m_vm, m_value);
            delete m_counter;
        }
    }

public:
    operator T() const { return m_value; }
    operator bool() const { return m_vm != nullptr; }

private:
    JavaVM* m_vm;
    std::function<void(JavaVM*, T)> m_deleter;
    T m_value;
    impl::ReferenceCounter* m_counter;
};

#endif  // com_mitsubishielectric_ahu_efw_cpucomservice_GlobalReference_h_
