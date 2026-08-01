package com.noty.geminimcp;

import javax.swing.*;
import java.awt.*;

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

        // Set system property for native access
        System.setProperty("java.awt.headless", "false");

        // Initialize native bridge with callback
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
        new Thread(() -> {
            try {
                Thread.sleep(500); // Give UI time to initialize
                NativeBridge.checkVSCode();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }).start();

        // Setup tray icon
        try {
            trayManager = new TrayManager(mainWindow);
            trayManager.setup();
        } catch (Exception e) {
            System.err.println("Failed to setup tray icon: " + e.getMessage());
        }

        // Shutdown hook
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            try {
                if (NativeBridge.isServerRunning()) {
                    NativeBridge.stopServer();
                }
                NativeBridge.shutdown();
            } catch (Exception e) {
                // Ignore
            }
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