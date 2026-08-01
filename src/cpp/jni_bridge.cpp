#include "jni_bridge.h"
#include "web_server.h"
#include "gemini_service.h"
#include "vscode_manager.h"
#include "encryption.h"
#include "config_manager.h"
#include "logger.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <thread>
#include <chrono>

// Static member initialization
JavaVM* JNIBridge::javaVM = nullptr;
jobject JNIBridge::javaCallback = nullptr;
std::unique_ptr<WebServer> JNIBridge::webServer;
std::unique_ptr<GeminiService> JNIBridge::geminiService;
std::unique_ptr<ConfigManager> JNIBridge::config;
std::unique_ptr<VSCodeManager> JNIBridge::vscodeManager;
std::unique_ptr<Logger> JNIBridge::logger;
bool JNIBridge::jvmInitialized = false;

bool JNIBridge::initialize(JavaVM* vm) {
    if (jvmInitialized) {
        logger->info("JVM already initialized");
        return true;
    }

    javaVM = vm;
    jvmInitialized = true;

    // Initialize components
    if (!logger) {
        logger = std::make_unique<Logger>("app.log");
    }
    if (!config) {
        config = std::make_unique<ConfigManager>();
    }
    if (!vscodeManager) {
        vscodeManager = std::make_unique<VSCodeManager>();
    }

    Encryption::init();

    if (!config->load()) {
        config->setDefaults();
    }

    logger->info("JNI Bridge initialized with JVM");
    return true;
}

void JNIBridge::setCallback(jobject callback) {
    if (!javaVM) {
        logger->error("JavaVM not initialized");
        return;
    }

    JNIEnv* env = nullptr;
    javaVM->AttachCurrentThread((void**)&env, nullptr);

    if (!env) {
        logger->error("Failed to attach thread to JVM");
        return;
    }

    if (javaCallback) {
        env->DeleteGlobalRef(javaCallback);
    }
    javaCallback = env->NewGlobalRef(callback);
    logger->info("Java callback set successfully");
}

void JNIBridge::sendToJava(const std::string& event, const std::string& data) {
    if (!javaCallback || !javaVM) {
        logger->warn("Cannot send to Java: callback not set");
        return;
    }

    JNIEnv* env = nullptr;
    javaVM->AttachCurrentThread((void**)&env, nullptr);

    if (!env) {
        logger->error("Failed to attach thread to JVM for callback");
        return;
    }

    jclass callbackClass = env->GetObjectClass(javaCallback);
    jmethodID methodId = env->GetMethodID(callbackClass, "onNativeEvent",
                                         "(Ljava/lang/String;Ljava/lang/String;)V");

    if (methodId) {
        jstring jEvent = env->NewStringUTF(event.c_str());
        jstring jData = env->NewStringUTF(data.c_str());
        env->CallVoidMethod(javaCallback, methodId, jEvent, jData);
        env->DeleteLocalRef(jEvent);
        env->DeleteLocalRef(jData);
    } else {
        logger->error("Failed to find onNativeEvent method");
    }
    env->DeleteLocalRef(callbackClass);
}

// JNI Method Implementations
extern "C" {

JNIEXPORT jboolean JNICALL Java_com_noty_geminimcp_NativeBridge_checkVSCode
(JNIEnv* env, jobject obj) {
    try {
        auto path = JNIBridge::vscodeManager->findInstallation();
        if (path.has_value()) {
            JNIBridge::config->setVSCodePath(path.value());
            nlohmann::json data;
            data["installed"] = true;
            data["path"] = path.value();
            JNIBridge::sendToJava("vscode-status", data.dump());
            JNIBridge::logger->info("VS Code found at: {}", path.value());
            return JNI_TRUE;
        } else {
            nlohmann::json data;
            data["installed"] = false;
            JNIBridge::sendToJava("vscode-status", data.dump());
            JNIBridge::logger->warn("VS Code not found");
            return JNI_FALSE;
        }
    } catch (const std::exception& e) {
        JNIBridge::logger->error("checkVSCode error: {}", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jstring JNICALL Java_com_noty_geminimcp_NativeBridge_getVSCodePath
(JNIEnv* env, jobject obj) {
    std::string path = JNIBridge::config->getVSCodePath();
    return env->NewStringUTF(path.c_str());
}

JNIEXPORT jboolean JNICALL Java_com_noty_geminimcp_NativeBridge_validateVSCodePath
(JNIEnv* env, jobject obj, jstring path) {
    const char* pathStr = env->GetStringUTFChars(path, nullptr);
    bool valid = JNIBridge::vscodeManager->validateExecutable(pathStr);
    env->ReleaseStringUTFChars(path, pathStr);
    return valid ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_com_noty_geminimcp_NativeBridge_saveApiKey
(JNIEnv* env, jobject obj, jstring key) {
    const char* keyStr = env->GetStringUTFChars(key, nullptr);
    try {
        std::string encrypted = Encryption::encrypt(keyStr);
        JNIBridge::config->setApiKey(encrypted);
        env->ReleaseStringUTFChars(key, keyStr);
        JNIBridge::logger->info("API key saved successfully");
        return JNI_TRUE;
    } catch (const std::exception& e) {
        JNIBridge::logger->error("Failed to save API key: {}", e.what());
        env->ReleaseStringUTFChars(key, keyStr);
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL Java_com_noty_geminimcp_NativeBridge_verifyApiKey
(JNIEnv* env, jobject obj, jstring key) {
    const char* keyStr = env->GetStringUTFChars(key, nullptr);
    try {
        JNIBridge::geminiService = std::make_unique<GeminiService>(keyStr);
        bool valid = JNIBridge::geminiService->verifyKey();
        env->ReleaseStringUTFChars(key, keyStr);

        nlohmann::json data;
        data["valid"] = valid;
        JNIBridge::sendToJava("api-key-verified", data.dump());

        if (valid) {
            JNIBridge::logger->info("API key verified successfully");
        } else {
            JNIBridge::logger->error("Invalid API key");
        }
        return valid ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        JNIBridge::logger->error("Failed to verify API key: {}", e.what());
        env->ReleaseStringUTFChars(key, keyStr);
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL Java_com_noty_geminimcp_NativeBridge_startServer
(JNIEnv* env, jobject obj) {
    try {
        if (!JNIBridge::geminiService || !JNIBridge::geminiService->isValid()) {
            JNIBridge::logger->error("Cannot start server: Gemini service not initialized");
            return JNI_FALSE;
        }

        // Stop existing server if running
        if (JNIBridge::webServer) {
            JNIBridge::webServer->stop();
            JNIBridge::webServer.reset();
        }

        JNIBridge::webServer = std::make_unique<WebServer>(31415);

        // Set up handlers
        JNIBridge::webServer->setChatHandler([](const std::string& msg, const std::string& ctx) {
            if (JNIBridge::geminiService && JNIBridge::geminiService->isValid()) {
                return JNIBridge::geminiService->sendMessage(msg, ctx);
            }
            return std::string("Error: Gemini service not initialized");
        });

        JNIBridge::webServer->setAnalyzeHandler([](const std::string& code, const std::string& question) {
            if (JNIBridge::geminiService && JNIBridge::geminiService->isValid()) {
                return JNIBridge::geminiService->analyzeCode(code, question);
            }
            return std::string("Error: Gemini service not initialized");
        });

        JNIBridge::webServer->setFixErrorsHandler([](const std::string& errorLog, const std::string& code) {
            if (JNIBridge::geminiService && JNIBridge::geminiService->isValid()) {
                return JNIBridge::geminiService->fixErrors(errorLog, code);
            }
            return std::string("Error: Gemini service not initialized");
        });

        if (JNIBridge::webServer->start()) {
            JNIBridge::config->setServerRunning(true);
            JNIBridge::sendToJava("server-started", "{}");
            JNIBridge::logger->info("Server started successfully on port 31415");
            return JNI_TRUE;
        }
        return JNI_FALSE;
    } catch (const std::exception& e) {
        JNIBridge::logger->error("Failed to start server: {}", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL Java_com_noty_geminimcp_NativeBridge_stopServer
(JNIEnv* env, jobject obj) {
    try {
        if (JNIBridge::webServer) {
            JNIBridge::webServer->stop();
            JNIBridge::webServer.reset();
        }
        JNIBridge::config->setServerRunning(false);
        JNIBridge::sendToJava("server-stopped", "{}");
        JNIBridge::logger->info("Server stopped");
        return JNI_TRUE;
    } catch (const std::exception& e) {
        JNIBridge::logger->error("Failed to stop server: {}", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL Java_com_noty_geminimcp_NativeBridge_isServerRunning
(JNIEnv* env, jobject obj) {
    return JNIBridge::config->getServerRunning() ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jstring JNICALL Java_com_noty_geminimcp_NativeBridge_sendChatMessage
(JNIEnv* env, jobject obj, jstring message, jstring context) {
    const char* msg = env->GetStringUTFChars(message, nullptr);
    const char* ctx = env->GetStringUTFChars(context, nullptr);

    std::string response;
    if (JNIBridge::geminiService && JNIBridge::geminiService->isValid()) {
        response = JNIBridge::geminiService->sendMessage(msg, ctx);
    } else {
        response = "Error: Gemini service not initialized";
    }

    env->ReleaseStringUTFChars(message, msg);
    env->ReleaseStringUTFChars(context, ctx);
    return env->NewStringUTF(response.c_str());
}

JNIEXPORT jstring JNICALL Java_com_noty_geminimcp_NativeBridge_analyzeCode
(JNIEnv* env, jobject obj, jstring code, jstring question) {
    const char* codeStr = env->GetStringUTFChars(code, nullptr);
    const char* qStr = env->GetStringUTFChars(question, nullptr);

    std::string response;
    if (JNIBridge::geminiService && JNIBridge::geminiService->isValid()) {
        response = JNIBridge::geminiService->analyzeCode(codeStr, qStr);
    } else {
        response = "Error: Gemini service not initialized";
    }

    env->ReleaseStringUTFChars(code, codeStr);
    env->ReleaseStringUTFChars(question, qStr);
    return env->NewStringUTF(response.c_str());
}

JNIEXPORT jstring JNICALL Java_com_noty_geminimcp_NativeBridge_fixErrors
(JNIEnv* env, jobject obj, jstring errorLog, jstring code) {
    const char* errorStr = env->GetStringUTFChars(errorLog, nullptr);
    const char* codeStr = env->GetStringUTFChars(code, nullptr);

    std::string response;
    if (JNIBridge::geminiService && JNIBridge::geminiService->isValid()) {
        response = JNIBridge::geminiService->fixErrors(errorStr, codeStr);
    } else {
        response = "Error: Gemini service not initialized";
    }

    env->ReleaseStringUTFChars(errorLog, errorStr);
    env->ReleaseStringUTFChars(code, codeStr);
    return env->NewStringUTF(response.c_str());
}

JNIEXPORT void JNICALL Java_com_noty_geminimcp_NativeBridge_shutdown
(JNIEnv* env, jobject obj) {
    if (JNIBridge::webServer) {
        JNIBridge::webServer->stop();
        JNIBridge::webServer.reset();
    }
    JNIBridge::geminiService.reset();
    JNIBridge::logger->info("Shutdown complete");
}

// Native method to start the application from C++
JNIEXPORT jboolean JNICALL Java_com_noty_geminimcp_NativeBridge_startApp
(JNIEnv* env, jobject obj) {
    JNIBridge::logger->info("Application started from C++");
    return JNI_TRUE;
}

} // extern "C"