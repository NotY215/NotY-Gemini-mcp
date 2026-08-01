package com.noty.geminimcp;

import java.io.File;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;

public class NativeBridge {
    static {
        try {
            // Try to load from libs directory first
            File libDir = new File("libs");
            if (libDir.exists()) {
                System.load(new File(libDir, "geminicore.dll").getAbsolutePath());
            } else {
                // Try to load from classpath (for packaged JAR)
                loadLibraryFromJar();
            }
            initializeNative();
        } catch (Exception e) {
            System.err.println("Failed to load native library: " + e.getMessage());
            e.printStackTrace();
        }
    }
    
    private static void loadLibraryFromJar() throws Exception {
        // Extract the DLL from the JAR and load it
        String libName = "geminicore.dll";
        File tempLib = File.createTempFile("geminicore", ".dll");
        tempLib.deleteOnExit();
        
        try (InputStream in = NativeBridge.class.getResourceAsStream("/libs/" + libName)) {
            if (in != null) {
                Files.copy(in, tempLib.toPath(), StandardCopyOption.REPLACE_EXISTING);
                System.load(tempLib.getAbsolutePath());
            } else {
                // Try to load from system path
                System.loadLibrary("geminicore");
            }
        }
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