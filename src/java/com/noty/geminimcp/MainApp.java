package com.noty.geminimcp;

import javax.swing.*;
import java.awt.*;
import java.nio.file.Paths;

public class MainApp {
    private static MainWindow mainWindow;
    private static TrayManager trayManager;
    
    public static void main(String[] args) {
        // Set look and feel
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        // Initialize native bridge
        NativeBridge.setCallback(new NativeBridge.NativeCallback() {
            @Override
            public void onNativeEvent(String event, String data) {
                SwingUtilities.invokeLater(() -> {
                    if (mainWindow != null) {
                        mainWindow.handleNativeEvent(event, data);
                    }
                });
            }
        });
        
        // Create main window
        mainWindow = new MainWindow();
        mainWindow.setVisible(true);
        
        // Check VS Code on startup
        NativeBridge.checkVSCode();
        
        // Setup tray icon
        trayManager = new TrayManager(mainWindow);
        trayManager.setup();
        
        // Shutdown hook
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            if (NativeBridge.isServerRunning()) {
                NativeBridge.stopServer();
            }
            NativeBridge.shutdown();
        }));
    }
    
    public static void showToast(String message, String type) {
        if (mainWindow != null) {
            mainWindow.showToast(message, type);
        }
    }
    
    public static void updateVSCodeStatus(boolean installed, String path) {
        if (mainWindow != null) {
            mainWindow.updateVSCodeStatus(installed, path);
        }
    }
    
    public static void updateApiKeyStatus(boolean valid) {
        if (mainWindow != null) {
            mainWindow.updateApiKeyStatus(valid);
        }
    }
    
    public static void updateServerStatus(boolean running) {
        if (mainWindow != null) {
            mainWindow.updateServerStatus(running);
        }
        if (trayManager != null) {
            trayManager.updateStatus(running);
        }
    }
}