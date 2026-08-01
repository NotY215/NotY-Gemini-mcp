#include "jni_bridge.h"
#include "web_server.h"
#include "gemini_service.h"
#include "vscode_manager.h"
#include "encryption.h"
#include "config_manager.h"
#include "logger.h"
#include <nlohmann/json.hpp>
#include <iostream>

// Static member initialization (now public)
JavaVM* JNIBridge::javaVM = nullptr;
jobject JNIBridge::javaCallback = nullptr;
std::unique_ptr<WebServer> JNIBridge::webServer;
std::unique_ptr<GeminiService> JNIBridge::geminiService;
std::unique_ptr<ConfigManager> JNIBridge::config;
std::unique_ptr<VSCodeManager> JNIBridge::vscodeManager;
std::unique_ptr<Logger> JNIBridge::logger;

bool JNIBridge::initialize(JavaVM* vm) {
    javaVM = vm;

    logger = std::make_unique<Logger>("app.log");
    config = std::make_unique<ConfigManager>();
    vscodeManager = std::make_unique<VSCodeManager>();
    Encryption::init();

    if (!config->load()) {
        config->setDefaults();
    }

    logger->info("JNI Bridge initialized");
    return true;
}

void JNIBridge::setCallback(jobject callback) {
    JNIEnv* env = nullptr;
    if (javaVM) {
        javaVM->AttachCurrentThread((void**)&env, nullptr);
    }

    if (javaCallback && env) {
        env->DeleteGlobalRef(javaCallback);
    }
    if (env && callback) {
        javaCallback = env->NewGlobalRef(callback);
    }
}

void JNIBridge::sendToJava(const std::string& event, const std::string& data) {
    if (!javaCallback || !javaVM) return;

    JNIEnv* env = nullptr;
    javaVM->AttachCurrentThread((void**)&env, nullptr);

    if (!env) return;

    jclass callbackClass = env->GetObjectClass(javaCallback);
    jmethodID methodId = env->GetMethodID(callbackClass, "onNativeEvent",
                                         "(Ljava/lang/String;Ljava/lang/String;)V");

    if (methodId) {
        jstring jEvent = env->NewStringUTF(event.c_str());
        jstring jData = env->NewStringUTF(data.c_str());
        env->CallVoidMethod(javaCallback, methodId, jEvent, jData);
        env->DeleteLocalRef(jEvent);
        env->DeleteLocalRef(jData);
    }
    env->DeleteLocalRef(callbackClass);
}

// JNI Method Implementations
extern "C" {

JNIEXPORT jboolean JNICALL Java_com_noty_geminimcp_NativeBridge_checkVSCode
(JNIEnv* env, jobject obj) {
    auto path = JNIBridge::vscodeManager->findInstallation();
    if (path.has_value()) {
        JNIBridge::config->setVSCodePath(path.value());
        nlohmann::json data;
        data["installed"] = true;
        data["path"] = path.value();
        JNIBridge::sendToJava("vscode-status", data.dump());
        return JNI_TRUE;
    } else {
        nlohmann::json data;
        data["installed"] = false;
        JNIBridge::sendToJava("vscode-status", data.dump());
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
        return JNI_TRUE;
    } catch (const std::exception& e) {
        if (JNIBridge::logger) {
            JNIBridge::logger->error("Failed to save API key: {}", e.what());
        }
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
        return valid ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        if (JNIBridge::logger) {
            JNIBridge::logger->error("Failed to verify API key: {}", e.what());
        }
        env->ReleaseStringUTFChars(key, keyStr);
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL Java_com_noty_geminimcp_NativeBridge_startServer
(JNIEnv* env, jobject obj) {
    try {
        if (!JNIBridge::geminiService || !JNIBridge::geminiService->isValid()) {
            if (JNIBridge::logger) {
                JNIBridge::logger->error("Cannot start server: Gemini service not initialized");
            }
            return JNI_FALSE;
        }

        JNIBridge::webServer = std::make_unique<WebServer>(31415);
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
            if (JNIBridge::logger) {
                JNIBridge::logger->info("Server started successfully");
            }
            return JNI_TRUE;
        }
        return JNI_FALSE;
    } catch (const std::exception& e) {
        if (JNIBridge::logger) {
            JNIBridge::logger->error("Failed to start server: {}", e.what());
        }
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
        if (JNIBridge::logger) {
            JNIBridge::logger->info("Server stopped");
        }
        return JNI_TRUE;
    } catch (const std::exception& e) {
        if (JNIBridge::logger) {
            JNIBridge::logger->error("Failed to stop server: {}", e.what());
        }
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
    if (JNIBridge::logger) {
        JNIBridge::logger->info("Shutdown complete");
    }
}

} // extern "C"