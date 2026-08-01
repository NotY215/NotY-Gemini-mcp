#include <jni.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "jni_bridge.h"
#include "logger.h"

// Global logger for C++ side
std::unique_ptr<Logger> g_logger;

// JNI function to initialize the bridge from Java
extern "C" JNIEXPORT jboolean JNICALL
Java_com_noty_geminimcp_NativeBridge_initializeNative(
    JNIEnv* env,
    jobject obj) {

    JavaVM* vm = nullptr;
    env->GetJavaVM(&vm);

    g_logger = std::make_unique<Logger>("cpp.log");
    g_logger->info("Native library loaded");

    return JNIBridge::initialize(vm) ? JNI_TRUE : JNI_FALSE;
}

#ifdef _WIN32
#include <windows.h>
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    (void)hinstDLL;
    (void)fdwReason;
    (void)lpvReserved;
    return TRUE;
}
#endif