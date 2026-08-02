package com.noty.geminimcp;

import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.net.URL;

public class MainWindow extends JFrame {
    private JPanel contentPanel;
    private JLabel statusBadge;
    private VSCodeSetupPanel vscodePanel;
    private ApiKeySetupPanel apiKeyPanel;
    private ServerControlPanel serverPanel;
    private ToastNotification toast;

    public MainWindow() {
        setTitle("⚡ NotY-Gemini-MCP");
        setSize(1000, 750);
        setMinimumSize(new Dimension(850, 650));
        setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
        setLocationRelativeTo(null);

        // Set icon
        ImageIcon icon = null;
        URL iconUrl = getClass().getResource("/icon.png");
        if (iconUrl != null) {
            icon = new ImageIcon(iconUrl);
        } else {
            try {
                icon = new ImageIcon("resources/icon.png");
            } catch (Exception e) {
                try {
                    icon = new ImageIcon("icon.png");
                } catch (Exception ex) {
                    // Use default
                }
            }
        }
        if (icon != null) {
            setIconImage(icon.getImage());
        }

        setupUI();
        setupWindowListeners();
        setupTheme();
    }

    private void setupUI() {
        setLayout(new BorderLayout(0, 0));

        // Main container with gradient background
        JPanel mainContainer = new JPanel(new BorderLayout(0, 0)) {
            @Override
            protected void paintComponent(Graphics g) {
                super.paintComponent(g);
                Graphics2D g2d = (Graphics2D) g;
                g2d.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);
                GradientPaint gp = new GradientPaint(0, 0, Theme.BG_PRIMARY, getWidth(), getHeight(), Theme.BG_SECONDARY);
                g2d.setPaint(gp);
                g2d.fillRect(0, 0, getWidth(), getHeight());
            }
        };
        mainContainer.setOpaque(false);
        mainContainer.setBorder(new EmptyBorder(20, 24, 20, 24));

        // Header
        JPanel headerPanel = createHeader();
        mainContainer.add(headerPanel, BorderLayout.NORTH);

        // Content with scroll
        JScrollPane scrollPane = new JScrollPane(createContentPanel());
        scrollPane.setBorder(null);
        scrollPane.getViewport().setOpaque(false);
        scrollPane.setOpaque(false);
        scrollPane.getVerticalScrollBar().setUnitIncrement(16);
        scrollPane.getVerticalScrollBar().setBackground(Theme.BG_SECONDARY);
        mainContainer.add(scrollPane, BorderLayout.CENTER);

        // Footer
        JPanel footerPanel = createFooter();
        mainContainer.add(footerPanel, BorderLayout.SOUTH);

        add(mainContainer, BorderLayout.CENTER);

        toast = new ToastNotification();
        setGlassPane(toast);
    }

    private JPanel createHeader() {
        JPanel header = new JPanel();
        header.setLayout(new BoxLayout(header, BoxLayout.Y_AXIS));
        header.setOpaque(false);
        header.setBorder(new EmptyBorder(0, 0, 20, 0));

        JLabel titleLabel = new JLabel("⚡ NotY-Gemini-MCP");
        titleLabel.setFont(new Font("Segoe UI", Font.BOLD, 34));
        titleLabel.setForeground(Theme.TEXT_PRIMARY);
        titleLabel.setAlignmentX(Component.CENTER_ALIGNMENT);
        header.add(titleLabel);

        JLabel subtitleLabel = new JLabel("🚀 AI-Powered Coding Assistant for VS Code");
        subtitleLabel.setFont(new Font("Segoe UI", Font.PLAIN, 16));
        subtitleLabel.setForeground(Theme.TEXT_SECONDARY);
        subtitleLabel.setAlignmentX(Component.CENTER_ALIGNMENT);
        header.add(Box.createRigidArea(new Dimension(0, 4)));
        header.add(subtitleLabel);

        // Status badge with glow effect
        JPanel badgeContainer = new JPanel(new FlowLayout(FlowLayout.CENTER, 0, 0));
        badgeContainer.setOpaque(false);

        statusBadge = new JLabel("● Server Stopped");
        statusBadge.setFont(new Font("Segoe UI", Font.BOLD, 13));
        statusBadge.setForeground(Theme.DANGER);
        statusBadge.setOpaque(true);
        statusBadge.setBackground(new Color(255, 107, 107, 30));
        statusBadge.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createLineBorder(new Color(255, 107, 107, 50), 1, true),
            new EmptyBorder(4, 20, 4, 20)
        ));
        badgeContainer.add(statusBadge);
        header.add(Box.createRigidArea(new Dimension(0, 12)));
        header.add(badgeContainer);

        return header;
    }

    private JPanel createContentPanel() {
        JPanel content = new JPanel();
        content.setLayout(new BoxLayout(content, BoxLayout.Y_AXIS));
        content.setOpaque(false);
        content.setBorder(new EmptyBorder(0, 0, 20, 0));

        vscodePanel = new VSCodeSetupPanel();
        content.add(vscodePanel);
        content.add(Box.createRigidArea(new Dimension(0, 16)));

        apiKeyPanel = new ApiKeySetupPanel();
        apiKeyPanel.setVisible(false);
        content.add(apiKeyPanel);
        content.add(Box.createRigidArea(new Dimension(0, 16)));

        serverPanel = new ServerControlPanel();
        serverPanel.setVisible(false);
        content.add(serverPanel);

        return content;
    }

    private JPanel createFooter() {
        JPanel footer = new JPanel();
        footer.setLayout(new BoxLayout(footer, BoxLayout.Y_AXIS));
        footer.setOpaque(false);
        footer.setBorder(new EmptyBorder(16, 0, 0, 0));

        JLabel footerLabel = new JLabel("💜 Made with love by NotY215/Fliczo | v1.0.0");
        footerLabel.setFont(new Font("Segoe UI", Font.PLAIN, 12));
        footerLabel.setForeground(Theme.TEXT_MUTED);
        footerLabel.setAlignmentX(Component.CENTER_ALIGNMENT);
        footer.add(footerLabel);

        JButton termsButton = new JButton("📜 View Terms & Conditions");
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
        UIManager.put("ScrollBar.thumb", Theme.BG_CARD);
        UIManager.put("ScrollBar.track", Theme.BG_SECONDARY);
    }

    private void setupWindowListeners() {
        addWindowListener(new WindowAdapter() {
            @Override
            public void windowClosing(WindowEvent e) {
                if (NativeBridge.isServerRunning()) {
                    setVisible(false);
                    MainApp.showToast("📌 Application minimized to system tray", "info");
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
            MainApp.showToast("✅ API key verified successfully!", "success");
        } else {
            MainApp.showToast("❌ Invalid API key. Please try again.", "error");
        }
    }

    public void updateServerStatus(boolean running) {
        serverPanel.setServerRunning(running);
        if (running) {
            statusBadge.setText("● Server Running");
            statusBadge.setForeground(Theme.SUCCESS);
            statusBadge.setBackground(new Color(72, 220, 120, 30));
            statusBadge.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createLineBorder(new Color(72, 220, 120, 50), 1, true),
                new EmptyBorder(4, 20, 4, 20)
            ));
            MainApp.showToast("🚀 Server started successfully!", "success");
        } else {
            statusBadge.setText("● Server Stopped");
            statusBadge.setForeground(Theme.DANGER);
            statusBadge.setBackground(new Color(255, 107, 107, 30));
            statusBadge.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createLineBorder(new Color(255, 107, 107, 50), 1, true),
                new EmptyBorder(4, 20, 4, 20)
            ));
            MainApp.showToast("⏹️ Server stopped.", "info");
        }
    }

    public void showToast(String message, String type) {
        toast.showToast(message, type);
    }
}