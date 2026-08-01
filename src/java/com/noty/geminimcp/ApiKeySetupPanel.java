package com.noty.geminimcp;

import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;
import java.awt.event.ActionListener;

public class ApiKeySetupPanel extends JPanel {
    private JPasswordField apiKeyField;
    private JButton saveButton;
    private JButton getKeyButton;
    private JLabel statusLabel;
    private JCheckBox showKeyCheck;
    private boolean valid;
    
    public ApiKeySetupPanel() {
        valid = false;
        setupUI();
    }
    
    private void setupUI() {
        setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
        setBackground(Theme.BG_CARD);
        setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createLineBorder(Theme.BORDER_COLOR, 1, true),
            new EmptyBorder(16, 16, 16, 16)
        ));
        setMaximumSize(new Dimension(Integer.MAX_VALUE, 220));
        setVisible(false);
        
        // Header
        JPanel headerPanel = new JPanel(new BorderLayout());
        headerPanel.setOpaque(false);
        
        JLabel titleLabel = new JLabel("🔑 Step 2: Gemini API Key");
        titleLabel.setFont(new Font("Segoe UI", Font.BOLD, 16));
        titleLabel.setForeground(Theme.TEXT_PRIMARY);
        headerPanel.add(titleLabel, BorderLayout.WEST);
        
        JLabel stepNumber = new JLabel("2");
        stepNumber.setFont(new Font("Segoe UI", Font.BOLD, 13));
        stepNumber.setForeground(Color.WHITE);
        stepNumber.setHorizontalAlignment(SwingConstants.CENTER);
        stepNumber.setOpaque(true);
        stepNumber.setBackground(Theme.ACCENT_PRIMARY);
        stepNumber.setBorder(new EmptyBorder(4, 10, 4, 10));
        headerPanel.add(stepNumber, BorderLayout.EAST);
        
        add(headerPanel);
        add(Box.createRigidArea(new Dimension(0, 12)));
        
        // Input
        JLabel inputLabel = new JLabel("Enter your Gemini API Key");
        inputLabel.setFont(new Font("Segoe UI", Font.PLAIN, 14));
        inputLabel.setForeground(Theme.TEXT_PRIMARY);
        add(inputLabel);
        add(Box.createRigidArea(new Dimension(0, 6)));
        
        JPanel inputPanel = new JPanel(new BorderLayout(8, 0));
        inputPanel.setOpaque(false);
        
        apiKeyField = new JPasswordField();
        apiKeyField.setFont(new Font("Segoe UI", Font.PLAIN, 14));
        apiKeyField.setForeground(Theme.TEXT_PRIMARY);
        apiKeyField.setBackground(new Color(0, 0, 0, 30));
        apiKeyField.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createLineBorder(Theme.BORDER_COLOR, 1, true),
            new EmptyBorder(8, 12, 8, 12)
        ));
        apiKeyField.setEchoChar('•');
        inputPanel.add(apiKeyField, BorderLayout.CENTER);
        
        showKeyCheck = new JCheckBox("👁");
        showKeyCheck.setFont(new Font("Segoe UI", Font.PLAIN, 14));
        showKeyCheck.setForeground(Theme.TEXT_MUTED);
        showKeyCheck.setBackground(null);
        showKeyCheck.setFocusPainted(false);
        showKeyCheck.addActionListener(e -> {
            if (showKeyCheck.isSelected()) {
                apiKeyField.setEchoChar((char)0);
            } else {
                apiKeyField.setEchoChar('•');
            }
        });
        inputPanel.add(showKeyCheck, BorderLayout.EAST);
        
        add(inputPanel);
        add(Box.createRigidArea(new Dimension(0, 12)));
        
        // Buttons
        JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10, 0));
        buttonPanel.setOpaque(false);
        
        saveButton = createButton("💾 Save & Verify", Theme.SUCCESS);
        saveButton.addActionListener(e -> {
            String key = new String(apiKeyField.getPassword()).trim();
            if (key.isEmpty()) {
                MainApp.showToast("Please enter a valid API key", "warning");
                return;
            }
            
            saveButton.setEnabled(false);
            saveButton.setText("⏳ Verifying...");
            MainApp.showToast("Verifying API key...", "info");
            
            new Thread(() -> {
                boolean saved = NativeBridge.saveApiKey(key);
                if (saved) {
                    boolean verified = NativeBridge.verifyApiKey(key);
                    SwingUtilities.invokeLater(() -> {
                        if (verified) {
                            valid = true;
                            statusLabel.setText("✅ API key verified successfully!");
                            statusLabel.setForeground(Theme.SUCCESS);
                            statusLabel.setVisible(true);
                            apiKeyField.setEnabled(false);
                            saveButton.setText("✓ Verified");
                            saveButton.setEnabled(false);
                            MainApp.updateApiKeyStatus(true);
                        } else {
                            statusLabel.setText("❌ Invalid API key. Please try again.");
                            statusLabel.setForeground(Theme.DANGER);
                            statusLabel.setVisible(true);
                            saveButton.setText("💾 Save & Verify");
                            saveButton.setEnabled(true);
                            MainApp.updateApiKeyStatus(false);
                        }
                    });
                } else {
                    SwingUtilities.invokeLater(() -> {
                        statusLabel.setText("❌ Failed to save API key");
                        statusLabel.setForeground(Theme.DANGER);
                        statusLabel.setVisible(true);
                        saveButton.setText("💾 Save & Verify");
                        saveButton.setEnabled(true);
                        MainApp.showToast("Failed to save API key", "error");
                    });
                }
            }).start();
        });
        buttonPanel.add(saveButton);
        
        getKeyButton = createLinkButton("🔑 Get API Key");
        getKeyButton.addActionListener(e -> {
            try {
                Desktop.getDesktop().browse(new java.net.URI("https://aistudio.google.com/api-keys"));
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        });
        buttonPanel.add(getKeyButton);
        
        buttonPanel.add(Box.createHorizontalGlue());
        add(buttonPanel);
        add(Box.createRigidArea(new Dimension(0, 8)));
        
        // Status
        statusLabel = new JLabel();
        statusLabel.setFont(new Font("Segoe UI", Font.PLAIN, 13));
        statusLabel.setVisible(false);
        add(statusLabel);
        add(Box.createRigidArea(new Dimension(0, 4)));
        
        JLabel hintLabel = new JLabel("🔒 Your API key is encrypted and stored locally");
        hintLabel.setFont(new Font("Segoe UI", Font.PLAIN, 12));
        hintLabel.setForeground(Theme.TEXT_MUTED);
        add(hintLabel);
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
    
    private JButton createLinkButton(String text) {
        JButton button = new JButton(text);
        button.setFont(new Font("Segoe UI", Font.BOLD, 13));
        button.setForeground(Theme.ACCENT_PRIMARY);
        button.setBackground(null);
        button.setBorder(new EmptyBorder(8, 16, 8, 16));
        button.setFocusPainted(false);
        button.setCursor(new Cursor(Cursor.HAND_CURSOR));
        button.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseEntered(java.awt.event.MouseEvent evt) {
                button.setForeground(Theme.ACCENT_PRIMARY.darker());
            }
            public void mouseExited(java.awt.event.MouseEvent evt) {
                button.setForeground(Theme.ACCENT_PRIMARY);
            }
        });
        return button;
    }
    
    public void setValid(boolean valid) {
        this.valid = valid;
        if (valid) {
            statusLabel.setText("✅ API key verified successfully!");
            statusLabel.setForeground(Theme.SUCCESS);
            statusLabel.setVisible(true);
            apiKeyField.setEnabled(false);
            saveButton.setText("✓ Verified");
            saveButton.setEnabled(false);
        } else {
            statusLabel.setText("❌ Invalid API key");
            statusLabel.setForeground(Theme.DANGER);
            statusLabel.setVisible(true);
            apiKeyField.setEnabled(true);
            saveButton.setText("💾 Save & Verify");
            saveButton.setEnabled(true);
        }
    }
}