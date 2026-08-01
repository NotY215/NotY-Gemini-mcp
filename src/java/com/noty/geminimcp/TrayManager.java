package com.noty.geminimcp;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.image.BufferedImage;
import java.net.URL;
import javax.imageio.ImageIO;
import javax.swing.*;

public class TrayManager {
    private TrayIcon trayIcon;
    private MainWindow mainWindow;
    private boolean isRunning;
    
    public TrayManager(MainWindow mainWindow) {
        this.mainWindow = mainWindow;
        this.isRunning = false;
    }
    
    public void setup() {
        if (!SystemTray.isSupported()) {
            System.out.println("System tray not supported");
            return;
        }
        
        try {
            // Load tray icon
            URL imageUrl = getClass().getResource("/resources/tray-icon.png");
            Image image;
            if (imageUrl != null) {
                image = ImageIO.read(imageUrl);
            } else {
                // Create a fallback icon
                image = createFallbackIcon();
            }
            
            // Scale image for tray
            Image scaledImage = image.getScaledInstance(16, 16, Image.SCALE_SMOOTH);
            
            // Create popup menu
            PopupMenu popup = new PopupMenu();
            
            MenuItem openItem = new MenuItem("Open NotY-Gemini-MCP");
            openItem.addActionListener(e -> {
                mainWindow.setVisible(true);
                mainWindow.toFront();
            });
            popup.add(openItem);
            
            popup.addSeparator();
            
            MenuItem statusItem = new MenuItem("Status: Stopped");
            statusItem.setEnabled(false);
            popup.add(statusItem);
            
            popup.addSeparator();
            
            MenuItem exitItem = new MenuItem("Exit");
            exitItem.addActionListener(e -> {
                if (NativeBridge.isServerRunning()) {
                    NativeBridge.stopServer();
                }
                NativeBridge.shutdown();
                System.exit(0);
            });
            popup.add(exitItem);
            
            // Create tray icon
            trayIcon = new TrayIcon(scaledImage, "NotY-Gemini-MCP", popup);
            trayIcon.setImageAutoSize(true);
            
            // Double click to show window
            trayIcon.addMouseListener(new MouseAdapter() {
                @Override
                public void mouseClicked(MouseEvent e) {
                    if (e.getClickCount() == 2) {
                        mainWindow.setVisible(true);
                        mainWindow.toFront();
                    }
                }
            });
            
            SystemTray.getSystemTray().add(trayIcon);
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    private Image createFallbackIcon() {
        BufferedImage image = new BufferedImage(16, 16, BufferedImage.TYPE_INT_ARGB);
        Graphics2D g = image.createGraphics();
        g.setColor(new Color(102, 126, 234));
        g.fillOval(0, 0, 16, 16);
        g.setColor(Color.WHITE);
        g.setFont(new Font("Arial", Font.BOLD, 10));
        g.drawString("G", 4, 12);
        g.dispose();
        return image;
    }
    
    public void updateStatus(boolean running) {
        this.isRunning = running;
        if (trayIcon != null) {
            String status = running ? "Running" : "Stopped";
            trayIcon.setToolTip("NotY-Gemini-MCP - Server " + status);
        }
    }
    
    public void showNotification(String message, String type) {
        if (trayIcon != null) {
            TrayIcon.MessageType msgType = TrayIcon.MessageType.INFO;
            if (type.equals("error")) {
                msgType = TrayIcon.MessageType.ERROR;
            } else if (type.equals("warning")) {
                msgType = TrayIcon.MessageType.WARNING;
            }
            trayIcon.displayMessage("NotY-Gemini-MCP", message, msgType);
        }
    }
}