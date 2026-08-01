package com.noty.geminimcp;

import java.io.File;
import java.nio.file.Files;
import java.nio.file.StandardCopyOption;
import java.io.InputStream;

public class NativeBridge {
    static {
        try {
            boolean loaded = false;

            // Try multiple locations
            String[] locations = {
                    "libs/geminicore.dll",
                    "geminicore.dll",
                    "build/classes/java/main/geminicore.dll",
                    "build/libs/geminicore.dll"
            };

            for (String loc : locations) {
                File libFile = new File(loc);
                if (libFile.exists()) {
                    System.load(libFile.getAbsolutePath());
                    System.out.println("Loaded geminicore.dll from: " + loc);
                    loaded = true;
                    break;
                }
            }

            if (!loaded) {
                // Try from classpath (JAR)
                try {
                    extractAndLoadLibrary();
                    loaded = true;
                } catch (Exception e) {
                    System.err.println("Could not extract library from JAR: " + e.getMessage());
                }
            }

            if (!loaded) {
                // Try system library path
                System.loadLibrary("geminicore");
                System.out.println("Loaded geminicore.dll from system path");
                loaded = true;
            }

            if (loaded) {
                initializeNative();
                System.out.println("Native bridge initialized successfully!");
            } else {
                System.err.println("Failed to load native library from any location");
                System.err.println("Make sure geminicore.dll and its dependencies are in the libs/ directory");
            }

        } catch (UnsatisfiedLinkError e) {
            System.err.println("Failed to load native library: " + e.getMessage());
            System.err.println("Make sure all dependent DLLs are present:");
            System.err.println("  - geminicore.dll");
            System.err.println("  - libcrypto-3-x64.dll");
            System.err.println("  - libcurl.dll");
            System.err.println("  - z.dll");
            System.err.println("  - spdlog.dll");
            System.err.println("  - fmt.dll");
            e.printStackTrace();
        } catch (Exception e) {
            System.err.println("Failed to initialize native bridge: " + e.getMessage());
            e.printStackTrace();
        }
    }

    private static void extractAndLoadLibrary() throws Exception {
        String libName = "geminicore.dll";
        File tempLib = File.createTempFile("geminicore", ".dll");
        tempLib.deleteOnExit();

        // Also extract dependencies
        String[] deps = {"libcrypto-3-x64.dll", "libcurl.dll", "z.dll", "spdlog.dll", "fmt.dll"};
        for (String dep : deps) {
            try (InputStream in = NativeBridge.class.getResourceAsStream("/libs/" + dep)) {
                if (in != null) {
                    File tempDep = File.createTempFile(dep.replace(".dll", ""), ".dll");
                    tempDep.deleteOnExit();
                    Files.copy(in, tempDep.toPath(), StandardCopyOption.REPLACE_EXISTING);
                }
            } catch (Exception e) {
                // Ignore - maybe not in JAR
            }
        }

        try (InputStream in = NativeBridge.class.getResourceAsStream("/libs/" + libName)) {
            if (in != null) {
                Files.copy(in, tempLib.toPath(), StandardCopyOption.REPLACE_EXISTING);
                System.load(tempLib.getAbsolutePath());
                System.out.println("Loaded geminicore.dll from JAR");
            } else {
                throw new Exception("Could not find geminicore.dll in JAR");
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