#include <jni.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <shlwapi.h>
#include "jni_bridge.h"
#include "logger.h"
#include <string>
#include <vector>

namespace fs = std::filesystem;

// Global Java VM
JavaVM* g_jvm = nullptr;
JNIEnv* g_env = nullptr;
std::unique_ptr<Logger> g_logger;

// Function to find Java installation
std::string findJavaHome() {
    std::vector<std::string> javaPaths;
    
    // Check JAVA_HOME environment variable
    const char* javaHome = std::getenv("JAVA_HOME");
    if (javaHome) {
        std::string path = std::string(javaHome) + "\\bin\\server\\jvm.dll";
        if (fs::exists(path)) {
            return std::string(javaHome);
        }
        path = std::string(javaHome) + "\\bin\\client\\jvm.dll";
        if (fs::exists(path)) {
            return std::string(javaHome);
        }
    }

    // Check registry for Java installations
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\JavaSoft\\Java Runtime Environment",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        char currentVersion[256];
        DWORD size = sizeof(currentVersion);
        if (RegQueryValueExA(hKey, "CurrentVersion", NULL, NULL,
            (LPBYTE)currentVersion, &size) == ERROR_SUCCESS) {

            std::string keyPath = "SOFTWARE\\JavaSoft\\Java Runtime Environment\\" + std::string(currentVersion);
            HKEY hSubKey;
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                char javaHomePath[1024];
                DWORD pathSize = sizeof(javaHomePath);
                if (RegQueryValueExA(hSubKey, "JavaHome", NULL, NULL,
                    (LPBYTE)javaHomePath, &pathSize) == ERROR_SUCCESS) {
                    RegCloseKey(hSubKey);
                    RegCloseKey(hKey);
                    return std::string(javaHomePath);
                }
                RegCloseKey(hSubKey);
            }
        }
        RegCloseKey(hKey);
    }

    // Check common Java installations
    std::vector<std::string> commonPaths = {
        "C:\\Program Files\\Eclipse Adoptium\\jdk-*",
        "C:\\Program Files\\Java\\jdk-*",
        "C:\\Program Files (x86)\\Java\\jdk-*",
        "C:\\Program Files\\OpenJDK\\*",
        "C:\\Program Files\\Amazon Corretto\\jdk*",
        "C:\\Program Files\\Microsoft\\jdk-*"
    };

    for (const auto& pattern : commonPaths) {
        std::string searchPath = pattern + "\\bin\\server\\jvm.dll";
        WIN32_FIND_DATAA findData;
        HANDLE findHandle = FindFirstFileA(searchPath.c_str(), &findData);
        if (findHandle != INVALID_HANDLE_VALUE) {
            std::string fullPath = pattern.substr(0, pattern.find('*'));
            fullPath += findData.cFileName;
            size_t pos = fullPath.find("\\bin\\server\\jvm.dll");
            if (pos != std::string::npos) {
                std::string home = fullPath.substr(0, pos);
                FindClose(findHandle);
                return home;
            }
            FindClose(findHandle);
        }
    }

    return "";
}

// Function to add Java bin to PATH
void addJavaBinToPath(const std::string& javaHome) {
    std::string javaBin = javaHome + "\\bin";
    std::string javaBinServer = javaHome + "\\bin\\server";
    std::string javaBinClient = javaHome + "\\bin\\client";

    char* currentPath = getenv("PATH");
    std::string newPath = javaBin + ";" + javaBinServer + ";" + javaBinClient;
    if (currentPath) {
        newPath += ";" + std::string(currentPath);
    }
    _putenv(("PATH=" + newPath).c_str());
}

// Function to start JVM
bool startJVM() {
    std::string javaHome = findJavaHome();
    if (javaHome.empty()) {
        std::cerr << "Java not found! Please install Java JDK." << std::endl;
        if (g_logger) {
            g_logger->error("Java not found!");
        }

        MessageBoxA(NULL,
            "Java Runtime Environment not found!\n\n"
            "Please install Java JDK 17 or later.\n"
            "Download from: https://adoptium.net/",
            "Java Not Found",
            MB_OK | MB_ICONERROR);
        return false;
    }

    std::cout << "Found Java at: " << javaHome << std::endl;
    if (g_logger) {
        g_logger->info("Found Java at: {}", javaHome);
    }

    addJavaBinToPath(javaHome);

    // Set JVM options
    JavaVMInitArgs vmArgs;
    JavaVMOption options[10];
    int optionCount = 0;

    std::string classpath = "-Djava.class.path=.";

    for (const auto& entry : fs::directory_iterator(".")) {
        if (entry.path().extension() == ".jar") {
            classpath += ";" + entry.path().string();
        }
    }

    if (fs::exists("libs")) {
        for (const auto& entry : fs::directory_iterator("libs")) {
            if (entry.path().extension() == ".jar") {
                classpath += ";libs/" + entry.path().filename().string();
            }
        }
    }

    if (fs::exists("classes")) {
        classpath += ";classes";
    }

    options[optionCount++].optionString = const_cast<char*>(classpath.c_str());
    options[optionCount++].optionString = const_cast<char*>("-Xmx512M");
    options[optionCount++].optionString = const_cast<char*>("-Djava.awt.headless=false");
    options[optionCount++].optionString = const_cast<char*>("--enable-native-access=ALL-UNNAMED");
    options[optionCount++].optionString = const_cast<char*>("-Djava.library.path=.");
    options[optionCount++].optionString = const_cast<char*>("-Djava.awt.systemTray=true");

    std::string jniClasspath = "-Djava.class.path=.;classes";
    options[optionCount++].optionString = const_cast<char*>(jniClasspath.c_str());

    vmArgs.version = JNI_VERSION_1_8;
    vmArgs.nOptions = optionCount;
    vmArgs.options = options;
    vmArgs.ignoreUnrecognized = JNI_FALSE;

    jint result = JNI_CreateJavaVM(&g_jvm, (void**)&g_env, &vmArgs);
    if (result != JNI_OK) {
        std::cerr << "Failed to create JVM. Error code: " << result << std::endl;
        if (g_logger) {
            g_logger->error("Failed to create JVM. Error code: {}", result);
        }

        if (result == JNI_ERR) {
            std::string errorMsg = "Failed to initialize Java Virtual Machine.\n\n"
                                  "Please ensure Java is properly installed.\n"
                                  "Error code: " + std::to_string(result);
            MessageBoxA(NULL, errorMsg.c_str(), "JVM Error", MB_OK | MB_ICONERROR);
        }
        return false;
    }

    std::cout << "JVM created successfully!" << std::endl;
    if (g_logger) {
        g_logger->info("JVM created successfully!");
    }
    return true;
}

// Function to run Java UI
bool runJavaUI() {
    if (!g_env || !g_jvm) {
        std::cerr << "JVM not initialized!" << std::endl;
        if (g_logger) {
            g_logger->error("JVM not initialized!");
        }
        return false;
    }

    jclass mainClass = g_env->FindClass("com/noty/geminimcp/MainApp");
    if (mainClass == nullptr) {
        std::cerr << "Failed to find MainApp class!" << std::endl;
        if (g_logger) {
            g_logger->error("Failed to find MainApp class!");
        }
        if (g_env->ExceptionCheck()) {
            g_env->ExceptionDescribe();
            g_env->ExceptionClear();
        }

        MessageBoxA(NULL,
            "Failed to load MainApp class!\n\n"
            "Make sure the JAR file is in the same directory.",
            "Class Not Found",
            MB_OK | MB_ICONERROR);
        return false;
    }

    jmethodID mainMethod = g_env->GetStaticMethodID(mainClass, "main", "([Ljava/lang/String;)V");
    if (mainMethod == nullptr) {
        std::cerr << "Failed to find main method!" << std::endl;
        if (g_logger) {
            g_logger->error("Failed to find main method!");
        }
        if (g_env->ExceptionCheck()) {
            g_env->ExceptionDescribe();
            g_env->ExceptionClear();
        }
        return false;
    }

    jclass stringClass = g_env->FindClass("java/lang/String");
    jobjectArray args = g_env->NewObjectArray(0, stringClass, nullptr);

    g_env->CallStaticVoidMethod(mainClass, mainMethod, args);

    if (g_env->ExceptionCheck()) {
        g_env->ExceptionDescribe();
        g_env->ExceptionClear();
        std::cerr << "Exception occurred while running Java UI!" << std::endl;
        if (g_logger) {
            g_logger->error("Exception occurred while running Java UI!");
        }
        return false;
    }

    std::cout << "Java UI launched successfully!" << std::endl;
    if (g_logger) {
        g_logger->info("Java UI launched successfully!");
    }
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
    std::cout << "  ⚡ NotY-Gemini-MCP v1.0.0" << std::endl;
    std::cout << "  (C) 2024 NotY215/Fliczo" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    g_logger = std::make_unique<Logger>("app.log");
    g_logger->info("Application starting...");

    char currentDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, currentDir);
    g_logger->info("Current directory: {}", currentDir);

    SetDllDirectoryA(currentDir);

    if (!startJVM()) {
        g_logger->error("Failed to start JVM!");
        FreeConsole();
        return 1;
    }

    if (!JNIBridge::initialize(g_jvm)) {
        g_logger->error("Failed to initialize JNI Bridge!");
        FreeConsole();
        return 1;
    }

    if (!runJavaUI()) {
        g_logger->error("Failed to run Java UI!");
        FreeConsole();
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (g_jvm) {
        g_jvm->DestroyJavaVM();
    }
    
    g_logger->info("Application shutting down...");
    FreeConsole();
    return 0;
}