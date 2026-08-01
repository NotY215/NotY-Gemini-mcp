package com.noty.geminimcp;

import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;
import java.awt.event.ActionListener;

public class VSCodeSetupPanel extends JPanel {
    private JLabel statusLabel;
    private JLabel pathLabel;
    private JButton browseButton;
    private JButton refreshButton;
    private JButton downloadButton;
    private boolean installed;
    private String installedPath;
    
    public VSCodeSetupPanel() {
        installed = false;
        installedPath = "";
        setupUI();
    }
    
    private void setupUI() {
        setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
        setBackground(Theme.BG_CARD);
        setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createLineBorder(Theme.BORDER_COLOR, 1, true),
            new EmptyBorder(16, 16, 16, 16)
        ));
        setMaximumSize(new Dimension(Integer.MAX_VALUE, 200));
        
        // Header
        JPanel headerPanel = new JPanel(new BorderLayout());
        headerPanel.setOpaque(false);
        
        JLabel titleLabel = new JLabel("📦 Step 1: VS Code Setup");
        titleLabel.setFont(new Font("Segoe UI", Font.BOLD, 16));
        titleLabel.setForeground(Theme.TEXT_PRIMARY);
        headerPanel.add(titleLabel, BorderLayout.WEST);
        
        JLabel stepNumber = new JLabel("1");
        stepNumber.setFont(new Font("Segoe UI", Font.BOLD, 13));
        stepNumber.setForeground(Color.WHITE);
        stepNumber.setHorizontalAlignment(SwingConstants.CENTER);
        stepNumber.setOpaque(true);
        stepNumber.setBackground(Theme.ACCENT_PRIMARY);
        stepNumber.setBorder(new EmptyBorder(4, 10, 4, 10));
        headerPanel.add(stepNumber, BorderLayout.EAST);
        
        add(headerPanel);
        add(Box.createRigidArea(new Dimension(0, 12)));
        
        // Status
        JPanel statusPanel = new JPanel(new BorderLayout());
        statusPanel.setOpaque(false);
        statusPanel.setBackground(new Color(0, 0, 0, 30));
        statusPanel.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createLineBorder(Theme.BORDER_COLOR, 1, true),
            new EmptyBorder(8, 12, 8, 12)
        ));
        
        statusLabel = new JLabel("❌ Checking VS Code installation...");
        statusLabel.setFont(new Font("Segoe UI", Font.PLAIN, 13));
        statusLabel.setForeground(Theme.TEXT_SECONDARY);
        statusPanel.add(statusLabel, BorderLayout.WEST);
        
        pathLabel = new JLabel();
        pathLabel.setFont(new Font("Segoe UI", Font.PLAIN, 11));
        pathLabel.setForeground(Theme.TEXT_MUTED);
        pathLabel.setVisible(false);
        statusPanel.add(pathLabel, BorderLayout.EAST);
        
        add(statusPanel);
        add(Box.createRigidArea(new Dimension(0, 12)));
        
        // Buttons
        JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.LEFT, 10, 0));
        buttonPanel.setOpaque(false);
        
        browseButton = createButton("📂 Browse", Theme.ACCENT_PRIMARY);
        browseButton.addActionListener(e -> {
            JFileChooser chooser = new JFileChooser("C:\\Program Files");
            chooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
            chooser.setDialogTitle("Select VS Code Executable");
            
            if (chooser.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
                String path = chooser.getSelectedFile().getAbsolutePath();
                if (NativeBridge.validateVSCodePath(path)) {
                    installedPath = path;
                    installed = true;
                    updateStatus();
                    MainApp.updateVSCodeStatus(true, path);
                } else {
                    MainApp.showToast("Invalid VS Code executable selected", "error");
                }
            }
        });
        buttonPanel.add(browseButton);
        
        refreshButton = createOutlinedButton("🔄 Refresh");
        refreshButton.addActionListener(e -> {
            MainApp.showToast("Checking VS Code installation...", "info");
            new Thread(() -> {
                boolean result = NativeBridge.checkVSCode();
                SwingUtilities.invokeLater(() -> {
                    if (!result) {
                        MainApp.showToast("VS Code not found", "warning");
                    }
                });
            }).start();
        });
        buttonPanel.add(refreshButton);
        
        downloadButton = createLinkButton("⬇ Download VS Code");
        downloadButton.addActionListener(e -> {
            try {
                Desktop.getDesktop().browse(new java.net.URI(
                    "https://code.visualstudio.com/download?_exp_download=fb315fc982"));
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        });
        buttonPanel.add(downloadButton);
        
        buttonPanel.add(Box.createHorizontalGlue());
        add(buttonPanel);
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
    
    private JButton createOutlinedButton(String text) {
        JButton button = new JButton(text);
        button.setFont(new Font("Segoe UI", Font.BOLD, 13));
        button.setForeground(Theme.TEXT_SECONDARY);
        button.setBackground(null);
        button.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createLineBorder(Theme.BORDER_COLOR, 1, true),
            new EmptyBorder(7, 19, 7, 19)
        ));
        button.setFocusPainted(false);
        button.setCursor(new Cursor(Cursor.HAND_CURSOR));
        button.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseEntered(java.awt.event.MouseEvent evt) {
                button.setBackground(new Color(255, 255, 255, 10));
            }
            public void mouseExited(java.awt.event.MouseEvent evt) {
                button.setBackground(null);
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
    
    public void setInstalled(boolean installed, String path) {
        this.installed = installed;
        this.installedPath = path;
        updateStatus();
    }
    
    private void updateStatus() {
        if (installed) {
            statusLabel.setText("✅ VS Code is installed");
            statusLabel.setForeground(Theme.SUCCESS);
            pathLabel.setText("Path: " + installedPath);
            pathLabel.setVisible(true);
        } else {
            statusLabel.setText("❌ VS Code is not installed");
            statusLabel.setForeground(Theme.DANGER);
            pathLabel.setVisible(false);
        }
    }
}