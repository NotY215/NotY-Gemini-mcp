package com.noty.geminimcp;

public class NativeBridge {
    static {
        System.loadLibrary("geminicore");
        initializeNative();
    }

    // Native methods
    public static native void initializeNative();
    public static native boolean checkVSCode();
    public static native String getVSCodePath();
    public static native boolean validateVSCodePath(String path);
    public static native boolean saveApiKey(String key);
    public static native boolean verifyApiKey(String key);
    public static native boolean startServer();
    public static native boolean stopServer();
    public static native boolean isServerRunning();
    public static native String sendChatMessage(String message, String context);
    public static native String analyzeCode(String code, String question);
    public static native String fixErrors(String errorLog, String code);
    public static native void shutdown();
    
    private static NativeCallback callback;
    
    public static void setCallback(NativeCallback cb) {
        callback = cb;
    }
    
    // Called from C++ side
    public static void onNativeEvent(String event, String data) {
        if (callback != null) {
            callback.onNativeEvent(event, data);
        }
    }
    
    public interface NativeCallback {
        void onNativeEvent(String event, String data);
    }
}