/*
 * COPYRIGHT (C) 2018 MITSUBISHI ELECTRIC CORPORATION
 * ALL RIGHTS RESERVED
 */

#ifndef _Included_com_mitsubishielectric_ahu_efw_cpucomservice_CpuComServiceImpl
#define _Included_com_mitsubishielectric_ahu_efw_cpucomservice_CpuComServiceImpl

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_mitsubishielectric_ahu_efw_cpucomservice_Client_create(JNIEnv*,
                                                                                       jobject,
                                                                                       jint);
JNIEXPORT void JNICALL Java_com_mitsubishielectric_ahu_efw_cpucomservice_Client_destroy(JNIEnv*,
                                                                                        jobject,
                                                                                        jint);
JNIEXPORT void JNICALL Java_com_mitsubishielectric_ahu_efw_cpucomservice_Client_send(JNIEnv*,
                                                                                     jobject,
                                                                                     jint,
                                                                                     jint,
                                                                                     jint,
                                                                                     jbyteArray);
JNIEXPORT void JNICALL
Java_com_mitsubishielectric_ahu_efw_cpucomservice_Client_setErrorCallback(JNIEnv*,
                                                                          jobject,
                                                                          jint,
                                                                          jobject);
JNIEXPORT void JNICALL Java_com_mitsubishielectric_ahu_efw_cpucomservice_Client_subscribe(JNIEnv*,
                                                                                          jobject,
                                                                                          jint,
                                                                                          jint,
                                                                                          jint,
                                                                                          jobject);
JNIEXPORT void JNICALL
Java_com_mitsubishielectric_ahu_efw_cpucomservice_Client_unsubscribe(JNIEnv*,
                                                                     jobject,
                                                                     jint,
                                                                     jint,
                                                                     jint,
                                                                     jobject);

#ifdef __cplusplus
}
#endif
#endif
