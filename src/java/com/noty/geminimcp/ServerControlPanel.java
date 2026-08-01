package com.noty.geminimcp;

import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

public class ServerControlPanel extends JPanel {
    private JButton startButton;
    private JButton stopButton;
    private JLabel statusLabel;
    private JTextArea logArea;
    private boolean serverRunning;

    public ServerControlPanel() {
        serverRunning = false;
        setupUI();
    }

    private void setupUI() {
        setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
        setBackground(Theme.BG_CARD);
        setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createLineBorder(Theme.BORDER_COLOR, 1, true),
                new EmptyBorder(16, 16, 16, 16)
        ));
        setMaximumSize(new Dimension(Integer.MAX_VALUE, 280));
        setVisible(false);

        // Header
        JPanel headerPanel = new JPanel(new BorderLayout());
        headerPanel.setOpaque(false);

        JLabel titleLabel = new JLabel("🚀 Step 3: MCP Server");
        titleLabel.setFont(new Font("Segoe UI", Font.BOLD, 16));
        titleLabel.setForeground(Theme.TEXT_PRIMARY);
        headerPanel.add(titleLabel, BorderLayout.WEST);

        JLabel stepNumber = new JLabel("3");
        stepNumber.setFont(new Font("Segoe UI", Font.BOLD, 13));
        stepNumber.setForeground(Color.WHITE);
        stepNumber.setHorizontalAlignment(SwingConstants.CENTER);
        stepNumber.setOpaque(true);
        stepNumber.setBackground(Theme.ACCENT_PRIMARY);
        stepNumber.setBorder(new EmptyBorder(4, 10, 4, 10));
        headerPanel.add(stepNumber, BorderLayout.EAST);

        add(headerPanel);
        add(Box.createRigidArea(new Dimension(0, 12)));

        // Buttons
        JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10, 0));
        buttonPanel.setOpaque(false);

        startButton = createButton("▶ Start Server", Theme.ACCENT_PRIMARY);
        startButton.addActionListener(e -> {
            startButton.setEnabled(false);
            startButton.setText("⏳ Starting...");
            MainApp.showToast("Starting server...", "info");
            addLog("⏳ Starting server...", "info");

            new Thread(() -> {
                boolean started = NativeBridge.startServer();
                SwingUtilities.invokeLater(() -> {
                    if (started) {
                        serverRunning = true;
                        updateServerUI();
                        MainApp.updateServerStatus(true);
                    } else {
                        startButton.setEnabled(true);
                        startButton.setText("▶ Start Server");
                        MainApp.showToast("Failed to start server", "error");
                        addLog("❌ Failed to start server", "error");
                    }
                });
            }).start();
        });
        buttonPanel.add(startButton);

        stopButton = createButton("⏹ Stop Server", Theme.DANGER);
        stopButton.setEnabled(false);
        stopButton.addActionListener(e -> {
            stopButton.setEnabled(false);
            MainApp.showToast("Stopping server...", "info");
            addLog("⏳ Stopping server...", "info");

            new Thread(() -> {
                boolean stopped = NativeBridge.stopServer();
                SwingUtilities.invokeLater(() -> {
                    if (stopped) {
                        serverRunning = false;
                        updateServerUI();
                        MainApp.updateServerStatus(false);
                    } else {
                        stopButton.setEnabled(true);
                        MainApp.showToast("Failed to stop server", "error");
                        addLog("❌ Failed to stop server", "error");
                    }
                });
            }).start();
        });
        buttonPanel.add(stopButton);

        buttonPanel.add(Box.createHorizontalGlue());
        add(buttonPanel);
        add(Box.createRigidArea(new Dimension(0, 8)));

        // Status
        statusLabel = new JLabel("Status: 🔴 Stopped");
        statusLabel.setFont(new Font("Segoe UI", Font.BOLD, 13));
        statusLabel.setForeground(Theme.DANGER);
        add(statusLabel);
        add(Box.createRigidArea(new Dimension(0, 4)));

        JLabel hintLabel = new JLabel("Server runs in background. Close window to minimize to tray.");
        hintLabel.setFont(new Font("Segoe UI", Font.PLAIN, 12));
        hintLabel.setForeground(Theme.TEXT_MUTED);
        add(hintLabel);
        add(Box.createRigidArea(new Dimension(0, 8)));

        // Log Area
        logArea = new JTextArea();
        logArea.setEditable(false);
        logArea.setFont(new Font("Consolas", Font.PLAIN, 12));
        logArea.setForeground(Theme.TEXT_SECONDARY);
        logArea.setBackground(new Color(0, 0, 0, 40));
        logArea.setBorder(BorderFactory.createCompoundBorder(
                BorderFactory.createLineBorder(Theme.BORDER_COLOR, 1, true),
                new EmptyBorder(8, 8, 8, 8)
        ));

        JScrollPane scrollPane = new JScrollPane(logArea);
        scrollPane.setBorder(null);
        scrollPane.setMaximumSize(new Dimension(Integer.MAX_VALUE, 120));
        scrollPane.setPreferredSize(new Dimension(Integer.MAX_VALUE, 120));
        add(scrollPane);

        // Add initial log
        addLog("🚀 Application initialized", "success");
        addLog("📋 Ready to start server", "info");
    }

    private JButton createButton(String text, Color bgColor) {
        JButton button = new JButton(text);
        button.setFont(new Font("Segoe UI", Font.BOLD, 13));
        button.setForeground(Color.WHITE);
        button.setBackground(bgColor);
        button.setBorder(new EmptyBorder(8, 20, 8, 20));
        button.setFocusPainted(false);
        button.setCursor(new Cursor(Cursor.HAND_CURSOR));
        button.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseEntered(java.awt.event.MouseEvent evt) {
                button.setBackground(button.getBackground().darker());
            }
            public void mouseExited(java.awt.event.MouseEvent evt) {
                button.setBackground(bgColor);
            }
        });
        return button;
    }

    private void updateServerUI() {
        if (serverRunning) {
            statusLabel.setText("Status: 🟢 Running");
            statusLabel.setForeground(Theme.SUCCESS);
            startButton.setEnabled(false);
            startButton.setText("▶ Running");
            stopButton.setEnabled(true);
        } else {
            statusLabel.setText("Status: 🔴 Stopped");
            statusLabel.setForeground(Theme.DANGER);
            startButton.setEnabled(true);
            startButton.setText("▶ Start Server");
            stopButton.setEnabled(false);
        }
    }

    public void setServerRunning(boolean running) {
        this.serverRunning = running;
        updateServerUI();
        if (running) {
            addLog("🚀 Server started successfully", "success");
        } else {
            addLog("⏹️ Server stopped", "info");
        }
    }

    private void addLog(String message, String type) {
        String timestamp = LocalDateTime.now().format(DateTimeFormatter.ofPattern("HH:mm:ss"));
        String formatted = String.format("[%s] ", timestamp);

        if (type.equals("success")) {
            formatted += "✅ " + message;
        } else if (type.equals("error")) {
            formatted += "❌ " + message;
        } else if (type.equals("warning")) {
            formatted += "⚠️ " + message;
        } else {
            formatted += "ℹ️ " + message;
        }

        logArea.append(formatted + "\n");
        logArea.setCaretPosition(logArea.getDocument().getLength());

        // Limit log lines
        String[] lines = logArea.getText().split("\n");
        if (lines.length > 100) {
            StringBuilder sb = new StringBuilder();
            for (int i = lines.length - 100; i < lines.length; i++) {
                sb.append(lines[i]).append("\n");
            }
            logArea.setText(sb.toString());
        }
    }
}