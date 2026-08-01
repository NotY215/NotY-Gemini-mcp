#include <jni.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>
#include "jni_bridge.h"
#include "logger.h"
#include <string>

namespace fs = std::filesystem;

// Global Java VM
JavaVM* g_jvm = nullptr;
JNIEnv* g_env = nullptr;

// Function to find Java installation
std::string findJavaHome() {
    // Check JAVA_HOME environment variable
    const char* javaHome = std::getenv("JAVA_HOME");
    if (javaHome) {
        std::string path = std::string(javaHome) + "\\bin\\server\\jvm.dll";
        if (fs::exists(path)) {
            return std::string(javaHome);
        }
    }

    // Check common Java installations
    std::vector<std::string> commonPaths = {
        "C:\\Program Files\\Eclipse Adoptium\\jdk-*\\bin\\server\\jvm.dll",
        "C:\\Program Files\\Java\\jdk-*\\bin\\server\\jvm.dll",
        "C:\\Program Files (x86)\\Java\\jdk-*\\bin\\server\\jvm.dll",
        "C:\\Program Files\\OpenJDK\\*\\bin\\server\\jvm.dll"
    };

    // Use Windows API to find Java
    WIN32_FIND_DATAA findData;
    HANDLE findHandle;

    for (const auto& pattern : commonPaths) {
        findHandle = FindFirstFileA(pattern.c_str(), &findData);
        if (findHandle != INVALID_HANDLE_VALUE) {
            std::string path = pattern.substr(0, pattern.find('*'));
            path += findData.cFileName;
            // Extract the JDK home
            size_t pos = path.find("\\bin\\server\\jvm.dll");
            if (pos != std::string::npos) {
                std::string home = path.substr(0, pos);
                if (fs::exists(path)) {
                    return home;
                }
            }
            FindClose(findHandle);
        }
    }

    return "";
}

// Function to start JVM
bool startJVM() {
    std::string javaHome = findJavaHome();
    if (javaHome.empty()) {
        std::cerr << "Java not found! Please install Java JDK." << std::endl;
        return false;
    }

    std::cout << "Found Java at: " << javaHome << std::endl;

    // Set JVM options
    JavaVMInitArgs vmArgs;
    JavaVMOption options[5];

    std::string classpath = "-Djava.class.path=.";
    // Add the Java classpath
    if (fs::exists("build/classes/java/main")) {
        classpath += ";build/classes/java/main";
    }
    if (fs::exists("libs/*")) {
        // Add JAR files from libs
        for (const auto& entry : fs::directory_iterator("libs")) {
            if (entry.path().extension() == ".jar") {
                classpath += ";" + entry.path().string();
            }
        }
    }

    options[0].optionString = const_cast<char*>(classpath.c_str());
    options[1].optionString = const_cast<char*>("-Xmx512M");
    options[2].optionString = const_cast<char*>("-Djava.awt.headless=false");
    options[3].optionString = const_cast<char*>("--enable-native-access=ALL-UNNAMED");
    options[4].optionString = const_cast<char*>("-Djava.library.path=.");

    vmArgs.version = JNI_VERSION_1_8;
    vmArgs.nOptions = 5;
    vmArgs.options = options;
    vmArgs.ignoreUnrecognized = JNI_TRUE;

    // Create JVM
    jint result = JNI_CreateJavaVM(&g_jvm, (void**)&g_env, &vmArgs);
    if (result != JNI_OK) {
        std::cerr << "Failed to create JVM. Error code: " << result << std::endl;
        return false;
    }

    std::cout << "JVM created successfully!" << std::endl;
    return true;
}

// Function to run Java UI
bool runJavaUI() {
    if (!g_env || !g_jvm) {
        std::cerr << "JVM not initialized!" << std::endl;
        return false;
    }

    // Find the MainApp class
    jclass mainClass = g_env->FindClass("com/noty/geminimcp/MainApp");
    if (mainClass == nullptr) {
        std::cerr << "Failed to find MainApp class!" << std::endl;
        if (g_env->ExceptionCheck()) {
            g_env->ExceptionDescribe();
            g_env->ExceptionClear();
        }
        return false;
    }

    // Find the main method
    jmethodID mainMethod = g_env->GetStaticMethodID(mainClass, "main", "([Ljava/lang/String;)V");
    if (mainMethod == nullptr) {
        std::cerr << "Failed to find main method!" << std::endl;
        if (g_env->ExceptionCheck()) {
            g_env->ExceptionDescribe();
            g_env->ExceptionClear();
        }
        return false;
    }

    // Create empty arguments array
    jobjectArray args = g_env->NewObjectArray(0, g_env->FindClass("java/lang/String"), nullptr);

    // Call main method
    g_env->CallStaticVoidMethod(mainClass, mainMethod, args);

    // Check for exceptions
    if (g_env->ExceptionCheck()) {
        g_env->ExceptionDescribe();
        g_env->ExceptionClear();
        std::cerr << "Exception occurred while running Java UI!" << std::endl;
        return false;
    }

    std::cout << "Java UI launched successfully!" << std::endl;
    return true;
}

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Allocate console for debugging
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);

    std::cout << "========================================" << std::endl;
    std::cout << "  NotY-Gemini-MCP v1.0.0" << std::endl;
    std::cout << "  (C) 2024 NotY215/Fliczo" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Initialize logger
    auto logger = std::make_unique<Logger>("app.log");
    logger->info("Application starting...");

    // Start JVM
    if (!startJVM()) {
        logger->error("Failed to start JVM!");
        MessageBoxA(NULL, "Failed to start Java Virtual Machine.\nPlease ensure Java is installed.",
                   "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    // Initialize JNI Bridge
    if (!JNIBridge::initialize(g_jvm)) {
        logger->error("Failed to initialize JNI Bridge!");
        return 1;
    }

    // Run Java UI
    if (!runJavaUI()) {
        logger->error("Failed to run Java UI!");
        return 1;
    }

    // Message loop for C++ (handles tray icon, etc.)
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup JVM
    if (g_jvm) {
        g_jvm->DestroyJavaVM();
    }

    logger->info("Application shutting down...");
    return 0;
}