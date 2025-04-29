/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#include "cpu_com_service_jni.h"
#include "GlobalReference.h"
#include <jni.h>

#include "CpuCom.h"
#include "libCpuCom.h"
#include "libMelcoCommon.h"

#include "ThreadPool.h"

#include <chrono>
#include <cstdint>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

using com::mitsubishielectric::ahu::common::CpuCommand;
using com::mitsubishielectric::ahu::common::ThreadPool;
using com::mitsubishielectric::ahu::cpucom::v2::ICpuCom;

using namespace com::mitsubishielectric::ahu::common;
using namespace com::mitsubishielectric::ahu::cpucom;

namespace {
// Java VM used to get an JNI interface
// NOTE: we do not create Java VM, we only attach our worker thread to the VM from the main thread,
// therefore, we call neither CreateJavaVM nor DestroyJavaVM.
JavaVM* gJavaVM = nullptr;

// JNI interface that is used to call Java methods from native code.
// Initialized at JNI_OnLoad along with a worker thread.
// NOTE: it should not be used directly, all Java calls should be done from the Java thread.
JNIEnv* gJNIEnv = nullptr;

// Pointers to the NativeCallbackWrapper/NativeErrorCallbackWrapper callback
// methods that are used to forward C++ calls to the java side. We cache these pointers in
// JNI_OnLoad call since calling FindClass from a native thread will use the "system" class loader
// and therefore will fail to find app-specific classes.
// FindClass calls made from JNI_OnLoad will use the class loader associated with the function that
// called System.loadLibrary and therefore it can find NativeClientWrapper class.
jmethodID gOnCommandMethod = nullptr;
jmethodID gOnErrorMethod = nullptr;

// Worker thread attached to Java VM used to call java methods.
std::shared_ptr<ThreadPool> gJavaThread;

// Each client of the CpuComService has its own ICpuCom instance associated with it by the process
// ID
std::map<jint, std::shared_ptr<v2::ICpuCom>> gClients;
std::mutex gClientsMutex;

std::map<jint, GlobalReference<jobject>> gErrorCallbacks;
std::mutex gErrorCallbacksMutex;

void callErrorCallback(int pid, CpuCommand command, int errorCode)
{
    GlobalReference<jobject> obj;
    {
        std::unique_lock<std::mutex> lock(gErrorCallbacksMutex);
        obj = gErrorCallbacks[pid];
    }
    if (obj) {
        auto l = [obj](CpuCommand command, int errorCode) {
            gJNIEnv->CallVoidMethod(obj, gOnErrorMethod, command.first, command.second, errorCode);
        };
        gJavaThread->push(l, command, errorCode);
    }
}

}  // namespace

jint JNI_OnLoad(JavaVM* jvm, void*)
{
    InitializeCommonLogMessages();
    InitializeLibCpuComLogMessages();
    gJavaVM = jvm;
    JNIEnv* env = nullptr;
    if (jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_OK) {
        jclass localClassReference =
            env->FindClass("com/mitsubishielectric/ahu/efw/cpucomservice/NativeCallbackWrapper");
        if (localClassReference) {
            gOnCommandMethod = env->GetMethodID(localClassReference, "onCommand", "(II[B)V");
        }
        else {
            return -1;
        }
        localClassReference = env->FindClass(
            "com/mitsubishielectric/ahu/efw/cpucomservice/NativeErrorCallbackWrapper");
        if (localClassReference) {
            gOnErrorMethod = env->GetMethodID(localClassReference, "onError", "(III)V");
        }
        else {
            return -1;
        }
    }
    else {
        return -1;
    }

    gJavaThread.reset(new ThreadPool(1));
    auto result = gJavaThread->push([jvm]() {
        JavaVMAttachArgs args = {JNI_VERSION_1_6, 0, 0};
        return jvm->AttachCurrentThread(&gJNIEnv, &args);
    });
    if (result.get() != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_6;
}

// This method is never actually called if the library is loaded by the default class loader which
// is never garbage-collected while retaining the reference to the library.
void JNI_OnUnload(JavaVM* jvm, void*)
{
    auto result = gJavaThread->push([jvm]() { return jvm->DetachCurrentThread(); });
    result.wait();

    gJavaThread.reset();
    gJavaVM = nullptr;
    TerminateLibCpuComLogMessages();
    TerminateCommonLogMessages();
}

JNIEXPORT void JNICALL Java_com_mitsubishielectric_ahu_efw_cpucomservice_Client_create(JNIEnv*,
                                                                                       jobject,
                                                                                       jint pid)
{
    auto errorCallback =
        std::bind(callErrorCallback, pid, std::placeholders::_1, std::placeholders::_2);
    std::unique_lock<std::mutex> lock(gClientsMutex);
    gClients.insert(std::make_pair(pid, std::shared_ptr<v2::ICpuCom>(v2::ICpuCom::create())));
    gClients[pid]->initialize(errorCallback, []() { /* do nothing*/ });
    gClients[pid]->connect();
}

JNIEXPORT void JNICALL Java_com_mitsubishielectric_ahu_efw_cpucomservice_Client_destroy(JNIEnv*,
                                                                                        jobject,
                                                                                        jint pid)
{
    std::unique_lock<std::mutex> lock(gClientsMutex);
    gClients[pid]->disconnect();
    gClients.erase(pid);
}

JNIEXPORT void JNICALL
Java_com_mitsubishielectric_ahu_efw_cpucomservice_Client_send(JNIEnv* env,
                                                              jobject,
                                                              jint pid,
                                                              jint command,
                                                              jint subCommand,
                                                              jbyteArray data)
{
    std::vector<uint8_t> nativeData;
    if (data != nullptr) {
        jsize length = env->GetArrayLength(data);
        jbyte* nativeBuffer = env->GetByteArrayElements(data, nullptr);
        nativeData.insert(nativeData.end(), nativeBuffer, nativeBuffer + length);
        env->ReleaseByteArrayElements(data, nativeBuffer, JNI_ABORT);
    }
    CpuCommand c = std::make_pair(command, subCommand);
    std::unique_lock<std::mutex> lock(gClientsMutex);
    gClients[pid]->send(c, nativeData);
}

JNIEXPORT void JNICALL
Java_com_mitsubishielectric_ahu_efw_cpucomservice_Client_setErrorCallback(JNIEnv* env,
                                                                          jobject,
                                                                          jint pid,
                                                                          jobject callback)
{
    std::unique_lock<std::mutex> lock(gErrorCallbacksMutex);
    if (env->IsSameObject(callback, nullptr)) {
        gErrorCallbacks[pid] = GlobalReference<jobject>();
    }
    else {
        gErrorCallbacks[pid] =
            GlobalReference<jobject>(callback, gJavaVM, [](JavaVM*, jobject obj) {
                gJavaThread->push([obj]() { gJNIEnv->DeleteGlobalRef(obj); });
            });
    }
}

JNIEXPORT void JNICALL
Java_com_mitsubishielectric_ahu_efw_cpucomservice_Client_subscribe(JNIEnv*,
                                                                   jobject,
                                                                   jint pid,
                                                                   jint command,
                                                                   jint subCommand,
                                                                   jobject callback)
{
    auto deleter = [](JavaVM*, jobject obj) {
        gJavaThread->push([obj]() { gJNIEnv->DeleteGlobalRef(obj); });
    };
    auto wrappedCallback = GlobalReference<jobject>(callback, gJavaVM, deleter);
    auto c = [wrappedCallback](CpuCommand command, std::vector<uint8_t> data) {
        jbyteArray jdata = gJNIEnv->NewByteArray(data.size());
        gJNIEnv->SetByteArrayRegion(jdata, 0, data.size(),
                                    reinterpret_cast<const jbyte*>(data.data()));
        gJNIEnv->CallVoidMethod(wrappedCallback, gOnCommandMethod, command.first, command.second,
                                jdata);
        gJNIEnv->DeleteLocalRef(jdata);
    };
    auto callable = [c](CpuCommand command, std::vector<uint8_t> data) {
        gJavaThread->push(c, std::move(command), std::move(data));
    };

    std::unique_lock<std::mutex> lock(gClientsMutex);
    gClients[pid]->subscribe(
        std::make_pair(static_cast<uint8_t>(command), static_cast<uint8_t>(subCommand)), callable);
}

JNIEXPORT void JNICALL
Java_com_mitsubishielectric_ahu_efw_cpucomservice_Client_unsubscribe(JNIEnv*,
                                                                     jobject,
                                                                     jint pid,
                                                                     jint command,
                                                                     jint subCommand,
                                                                     jobject)
{
    std::unique_lock<std::mutex> lock(gClientsMutex);
    gClients[pid]->unsubscribe(
        std::make_pair(static_cast<uint8_t>(command), static_cast<uint8_t>(subCommand)));
}
