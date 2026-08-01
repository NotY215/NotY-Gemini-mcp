package com.noty.geminimcp;

import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class ToastNotification extends JComponent {
    private JPanel toastPanel;
    private JLabel messageLabel;
    private Timer timer;
    private String currentType = "info";
    
    public ToastNotification() {
        setLayout(new GridBagLayout());
        setOpaque(false);
        setVisible(false);
        
        toastPanel = new JPanel();
        toastPanel.setLayout(new BorderLayout());
        toastPanel.setBorder(new EmptyBorder(14, 24, 14, 24));
        toastPanel.setOpaque(true);
        
        messageLabel = new JLabel();
        messageLabel.setFont(new Font("Segoe UI", Font.BOLD, 14));
        messageLabel.setForeground(Color.WHITE);
        toastPanel.add(messageLabel, BorderLayout.CENTER);
        
        setToastColor("info");
    }
    
    private void setToastColor(String type) {
        Color bgColor;
        Color borderColor;
        
        switch (type) {
            case "success":
                bgColor = new Color(72, 187, 120, 230);
                borderColor = new Color(72, 187, 120, 80);
                break;
            case "error":
                bgColor = new Color(245, 101, 101, 230);
                borderColor = new Color(245, 101, 101, 80);
                break;
            case "warning":
                bgColor = new Color(246, 173, 85, 230);
                borderColor = new Color(246, 173, 85, 80);
                break;
            default:
                bgColor = new Color(66, 153, 225, 230);
                borderColor = new Color(66, 153, 225, 80);
                break;
        }
        
        toastPanel.setBackground(bgColor);
        toastPanel.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createLineBorder(borderColor, 1, true),
            new EmptyBorder(14, 24, 14, 24)
        ));
    }
    
    public void showToast(String message, String type) {
        currentType = type;
        setToastColor(type);
        messageLabel.setText(message);
        
        // Position at bottom center
        removeAll();
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.gridx = 0;
        gbc.gridy = 0;
        gbc.weightx = 1.0;
        gbc.weighty = 1.0;
        gbc.anchor = GridBagConstraints.SOUTH;
        gbc.insets = new Insets(0, 20, 40, 20);
        add(toastPanel, gbc);
        
        setVisible(true);
        revalidate();
        repaint();
        
        // Auto-hide after 3 seconds
        if (timer != null) {
            timer.stop();
        }
        timer = new Timer(3000, e -> {
            setVisible(false);
            timer.stop();
        });
        timer.setRepeats(false);
        timer.start();
    }
    
    @Override
    protected void paintComponent(Graphics g) {
        // Make glass pane transparent
        g.setColor(new Color(0, 0, 0, 0));
        g.fillRect(0, 0, getWidth(), getHeight());
    }
}