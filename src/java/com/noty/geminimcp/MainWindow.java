package com.noty.geminimcp;

import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

public class MainWindow extends JFrame {
    private JPanel contentPanel;
    private JLabel statusBadge;
    private VSCodeSetupPanel vscodePanel;
    private ApiKeySetupPanel apiKeyPanel;
    private ServerControlPanel serverPanel;
    private ToastNotification toast;
    
    public MainWindow() {
        setTitle("NotY-Gemini-MCP");
        setSize(920, 700);
        setMinimumSize(new Dimension(800, 600));
        setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        setLocationRelativeTo(null);
        setIconImage(Toolkit.getDefaultToolkit().getImage(
            getClass().getResource("/resources/icon.png")));
        
        setupUI();
        setupWindowListeners();
        setupTheme();
    }
    
    private void setupUI() {
        setLayout(new BorderLayout(0, 0));
        
        // Main container with padding
        JPanel mainContainer = new JPanel(new BorderLayout(0, 0));
        mainContainer.setBackground(Theme.BG_SECONDARY);
        mainContainer.setBorder(new EmptyBorder(20, 20, 20, 20));
        
        // Header
        JPanel headerPanel = createHeader();
        mainContainer.add(headerPanel, BorderLayout.NORTH);
        
        // Content with scroll
        JScrollPane scrollPane = new JScrollPane(createContentPanel());
        scrollPane.setBorder(null);
        scrollPane.getViewport().setBackground(Theme.BG_SECONDARY);
        scrollPane.getVerticalScrollBar().setUnitIncrement(16);
        mainContainer.add(scrollPane, BorderLayout.CENTER);
        
        // Footer
        JPanel footerPanel = createFooter();
        mainContainer.add(footerPanel, BorderLayout.SOUTH);
        
        add(mainContainer, BorderLayout.CENTER);
        
        // Toast notification (floating)
        toast = new ToastNotification();
        setGlassPane(toast);
    }
    
    private JPanel createHeader() {
        JPanel header = new JPanel();
        header.setLayout(new BoxLayout(header, BoxLayout.Y_AXIS));
        header.setBackground(Theme.BG_SECONDARY);
        header.setBorder(new EmptyBorder(0, 0, 16, 0));
        
        JLabel titleLabel = new JLabel("⚡ NotY-Gemini-MCP");
        titleLabel.setFont(new Font("Segoe UI", Font.BOLD, 28));
        titleLabel.setForeground(Theme.TEXT_PRIMARY);
        titleLabel.setAlignmentX(Component.CENTER_ALIGNMENT);
        header.add(titleLabel);
        
        JLabel subtitleLabel = new JLabel("AI-Powered Coding Assistant for VS Code");
        subtitleLabel.setFont(new Font("Segoe UI", Font.PLAIN, 14));
        subtitleLabel.setForeground(Theme.TEXT_SECONDARY);
        subtitleLabel.setAlignmentX(Component.CENTER_ALIGNMENT);
        header.add(subtitleLabel);
        
        // Status badge
        statusBadge = new JLabel("● Server Stopped");
        statusBadge.setFont(new Font("Segoe UI", Font.BOLD, 12));
        statusBadge.setForeground(Theme.DANGER);
        statusBadge.setBackground(new Color(245, 101, 101, 30));
        statusBadge.setOpaque(true);
        statusBadge.setBorder(new EmptyBorder(4, 16, 4, 16));
        statusBadge.setAlignmentX(Component.CENTER_ALIGNMENT);
        statusBadge.setMaximumSize(new Dimension(200, 30));
        header.add(Box.createRigidArea(new Dimension(0, 8)));
        header.add(statusBadge);
        
        return header;
    }
    
    private JPanel createContentPanel() {
        JPanel content = new JPanel();
        content.setLayout(new BoxLayout(content, BoxLayout.Y_AXIS));
        content.setBackground(Theme.BG_SECONDARY);
        content.setBorder(new EmptyBorder(0, 0, 20, 0));
        
        // Step 1: VS Code Setup
        vscodePanel = new VSCodeSetupPanel();
        content.add(vscodePanel);
        content.add(Box.createRigidArea(new Dimension(0, 16)));
        
        // Step 2: API Key Setup
        apiKeyPanel = new ApiKeySetupPanel();
        apiKeyPanel.setVisible(false);
        content.add(apiKeyPanel);
        content.add(Box.createRigidArea(new Dimension(0, 16)));
        
        // Step 3: Server Control
        serverPanel = new ServerControlPanel();
        serverPanel.setVisible(false);
        content.add(serverPanel);
        
        return content;
    }
    
    private JPanel createFooter() {
        JPanel footer = new JPanel();
        footer.setLayout(new BoxLayout(footer, BoxLayout.Y_AXIS));
        footer.setBackground(Theme.BG_SECONDARY);
        footer.setBorder(new EmptyBorder(12, 0, 0, 0));
        
        JLabel footerLabel = new JLabel("NotY-Gemini-MCP v1.0.0 | Made with ❤️ by NotY215/Fliczo");
        footerLabel.setFont(new Font("Segoe UI", Font.PLAIN, 12));
        footerLabel.setForeground(Theme.TEXT_MUTED);
        footerLabel.setAlignmentX(Component.CENTER_ALIGNMENT);
        footer.add(footerLabel);
        
        JButton termsButton = new JButton("View Terms & Conditions");
        termsButton.setFont(new Font("Segoe UI", Font.PLAIN, 11));
        termsButton.setForeground(Theme.ACCENT_PRIMARY);
        termsButton.setBorderPainted(false);
        termsButton.setContentAreaFilled(false);
        termsButton.setCursor(new Cursor(Cursor.HAND_CURSOR));
        termsButton.setAlignmentX(Component.CENTER_ALIGNMENT);
        termsButton.addActionListener(e -> new TermsDialog(this).setVisible(true));
        footer.add(termsButton);
        
        return footer;
    }
    
    private void setupTheme() {
        // Set dark theme for scrollbars
        UIManager.put("ScrollBar.thumb", Theme.BG_CARD);
        UIManager.put("ScrollBar.track", Theme.BG_SECONDARY);
    }
    
    private void setupWindowListeners() {
        addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e) {
                if (NativeBridge.isServerRunning()) {
                    setVisible(false);
                    MainApp.showToast("Application minimized to system tray", "info");
                } else {
                    System.exit(0);
                }
            }
        });
    }
    
    public void handleNativeEvent(String event, String data) {
        switch (event) {
            case "vscode-status":
                try {
                    com.google.gson.JsonObject json = com.google.gson.JsonParser.parseString(data).getAsJsonObject();
                    boolean installed = json.get("installed").getAsBoolean();
                    String path = json.has("path") ? json.get("path").getAsString() : "";
                    updateVSCodeStatus(installed, path);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                break;
                
            case "api-key-verified":
                try {
                    com.google.gson.JsonObject json = com.google.gson.JsonParser.parseString(data).getAsJsonObject();
                    boolean valid = json.get("valid").getAsBoolean();
                    updateApiKeyStatus(valid);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                break;
                
            case "server-started":
                updateServerStatus(true);
                break;
                
            case "server-stopped":
                updateServerStatus(false);
                break;
        }
    }
    
    public void updateVSCodeStatus(boolean installed, String path) {
        vscodePanel.setInstalled(installed, path);
        apiKeyPanel.setVisible(installed);
        if (!installed) {
            apiKeyPanel.setVisible(false);
            serverPanel.setVisible(false);
        }
    }
    
    public void updateApiKeyStatus(boolean valid) {
        apiKeyPanel.setValid(valid);
        serverPanel.setVisible(valid);
        if (valid) {
            MainApp.showToast("API key verified successfully!", "success");
        } else {
            MainApp.showToast("Invalid API key. Please check and try again.", "error");
        }
    }
    
    public void updateServerStatus(boolean running) {
        serverPanel.setServerRunning(running);
        if (running) {
            statusBadge.setText("● Server Running");
            statusBadge.setForeground(Theme.SUCCESS);
            statusBadge.setBackground(new Color(72, 187, 120, 30));
            MainApp.showToast("Server started successfully!", "success");
        } else {
            statusBadge.setText("● Server Stopped");
            statusBadge.setForeground(Theme.DANGER);
            statusBadge.setBackground(new Color(245, 101, 101, 30));
            MainApp.showToast("Server stopped.", "info");
        }
    }
    
    public void showToast(String message, String type) {
        toast.showToast(message, type);
    }
}