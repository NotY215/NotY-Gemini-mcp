#pragma once
#include <jni.h>
#include <string>
#include <functional>
#include <memory>

// Forward declarations
class WebServer;
class GeminiService;
class ConfigManager;
class VSCodeManager;
class Logger;

class JNIBridge {
public:
    // Static members
    static JavaVM* javaVM;
    static jobject javaCallback;
    static std::unique_ptr<WebServer> webServer;
    static std::unique_ptr<GeminiService> geminiService;
    static std::unique_ptr<ConfigManager> config;
    static std::unique_ptr<VSCodeManager> vscodeManager;
    static std::unique_ptr<Logger> logger;
    static bool jvmInitialized;

    static bool initialize(JavaVM* vm);
    static void setCallback(jobject callback);
    static void sendToJava(const std::string& event, const std::string& data);

    // Native methods called from Java
    static jboolean JNICALL checkVSCode(JNIEnv* env, jobject obj);
    static jstring JNICALL getVSCodePath(JNIEnv* env, jobject obj);
    static jboolean JNICALL validateVSCodePath(JNIEnv* env, jobject obj, jstring path);
    static jboolean JNICALL saveApiKey(JNIEnv* env, jobject obj, jstring key);
    static jboolean JNICALL verifyApiKey(JNIEnv* env, jobject obj, jstring key);
    static jboolean JNICALL startServer(JNIEnv* env, jobject obj);
    static jboolean JNICALL stopServer(JNIEnv* env, jobject obj);
    static jboolean JNICALL isServerRunning(JNIEnv* env, jobject obj);
    static jstring JNICALL sendChatMessage(JNIEnv* env, jobject obj, jstring message, jstring context);
    static jstring JNICALL analyzeCode(JNIEnv* env, jobject obj, jstring code, jstring question);
    static jstring JNICALL fixErrors(JNIEnv* env, jobject obj, jstring errorLog, jstring code);
    static void JNICALL shutdown(JNIEnv* env, jobject obj);
    static jboolean JNICALL startApp(JNIEnv* env, jobject obj);
};