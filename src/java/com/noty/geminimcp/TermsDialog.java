package com.noty.geminimcp;

import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class TermsDialog extends JDialog {
    private boolean accepted = false;
    
    public TermsDialog(JFrame parent) {
        super(parent, "Terms & Conditions", true);
        setupUI();
        setSize(700, 600);
        setLocationRelativeTo(parent);
        setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);
    }
    
    private void setupUI() {
        JPanel mainPanel = new JPanel(new BorderLayout(0, 0));
        mainPanel.setBackground(Theme.BG_SECONDARY);
        mainPanel.setBorder(new EmptyBorder(24, 24, 24, 24));
        
        // Header
        JPanel headerPanel = new JPanel(new BorderLayout());
        headerPanel.setOpaque(false);
        headerPanel.setBorder(new EmptyBorder(0, 0, 16, 0));
        
        JLabel titleLabel = new JLabel("📜 Terms & Conditions");
        titleLabel.setFont(new Font("Segoe UI", Font.BOLD, 24));
        titleLabel.setForeground(Theme.TEXT_PRIMARY);
        headerPanel.add(titleLabel, BorderLayout.WEST);
        
        JLabel versionLabel = new JLabel("NotY-Gemini-MCP v1.0.0");
        versionLabel.setFont(new Font("Segoe UI", Font.PLAIN, 13));
        versionLabel.setForeground(Theme.TEXT_MUTED);
        headerPanel.add(versionLabel, BorderLayout.EAST);
        
        mainPanel.add(headerPanel, BorderLayout.NORTH);
        
        // Terms content with scroll
        JTextArea termsArea = new JTextArea();
        termsArea.setEditable(false);
        termsArea.setFont(new Font("Segoe UI", Font.PLAIN, 13));
        termsArea.setForeground(Theme.TEXT_SECONDARY);
        termsArea.setBackground(new Color(0, 0, 0, 30));
        termsArea.setBorder(new EmptyBorder(12, 12, 12, 12));
        termsArea.setText(getTermsText());
        termsArea.setCaretPosition(0);
        
        JScrollPane scrollPane = new JScrollPane(termsArea);
        scrollPane.setBorder(BorderFactory.createCompoundBorder(
            BorderFactory.createLineBorder(Theme.BORDER_COLOR, 1, true),
            new EmptyBorder(0, 0, 0, 0)
        ));
        scrollPane.setPreferredSize(new Dimension(650, 350));
        mainPanel.add(scrollPane, BorderLayout.CENTER);
        
        // Footer buttons
        JPanel buttonPanel = new JPanel(new FlowLayout(FlowLayout.CENTER, 12, 16));
        buttonPanel.setOpaque(false);
        
        JButton acceptButton = new JButton("✅ I Accept");
        acceptButton.setFont(new Font("Segoe UI", Font.BOLD, 14));
        acceptButton.setForeground(Color.WHITE);
        acceptButton.setBackground(Theme.ACCENT_PRIMARY);
        acceptButton.setBorder(new EmptyBorder(10, 30, 10, 30));
        acceptButton.setFocusPainted(false);
        acceptButton.setCursor(new Cursor(Cursor.HAND_CURSOR));
        acceptButton.addActionListener(e -> {
            accepted = true;
            dispose();
        });
        buttonPanel.add(acceptButton);
        
        JButton declineButton = new JButton("❌ I Decline");
        declineButton.setFont(new Font("Segoe UI", Font.BOLD, 14));
        declineButton.setForeground(Color.WHITE);
        declineButton.setBackground(Theme.DANGER);
        declineButton.setBorder(new EmptyBorder(10, 30, 10, 30));
        declineButton.setFocusPainted(false);
        declineButton.setCursor(new Cursor(Cursor.HAND_CURSOR));
        declineButton.addActionListener(e -> {
            accepted = false;
            dispose();
            System.exit(0);
        });
        buttonPanel.add(declineButton);
        
        mainPanel.add(buttonPanel, BorderLayout.SOUTH);
        
        add(mainPanel);
    }
    
    private String getTermsText() {
        return """
            ⚠️ DISCLAIMER OF LIABILITY
            The developers and contributors of NotY-Gemini-MCP are NOT responsible for any data breaches,
            security incidents, or unauthorized access to your systems. This software is provided "AS IS"
            and you use it entirely at your own risk.
            
            1. ACCEPTANCE OF TERMS
            By installing, downloading, or using NotY-Gemini-MCP, you agree to comply with and be bound by
            these Terms & Conditions. If you do not agree to these terms, you must immediately uninstall
            and discontinue use of the Software.
            
            2. DATA HANDLING AND PRIVACY
            Your Gemini API key is encrypted using AES-256 encryption and stored locally in
            %appdata%/gemini-mcp/key.db. The encryption key is derived from your system's unique identifiers.
            
            When you use the Software to interact with Google's Gemini AI, the following data may be
            transmitted to Google's servers:
            • Your Gemini API key (for authentication)
            • Code snippets and project files you choose to analyze
            • Natural language queries and conversations
            • Error logs and debugging information
            
            All data transmission occurs over encrypted HTTPS connections to Google's API endpoints.
            
            3. SECURITY RISKS
            Using AI-powered coding assistants involves inherent security risks, including but not limited to:
            • Data Exposure: Code snippets processed by AI may be temporarily stored by Google's servers
            • Intellectual Property: Sharing proprietary code with AI services may have implications
            • API Key Compromise: If your API key is exposed, unauthorized users could access your quota
            • Man-in-the-Middle Attacks: While encrypted, there is always a theoretical risk
            • Malicious Code Suggestions: AI-generated code could potentially contain vulnerabilities
            
            4. USER OBLIGATIONS
            By using the Software, you agree to:
            • Keep your Gemini API key secure and not share it with others
            • Review all AI-generated code suggestions before implementing them
            • Not use the Software for illegal or malicious purposes
            • Ensure you have the rights to analyze any code you submit
            • Regularly audit your API key usage and rotate keys periodically
            • Not upload sensitive credentials, passwords, or private keys to the AI service
            
            5. COMPLETE DISCLAIMER OF LIABILITY
            TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THE DEVELOPERS SHALL NOT BE LIABLE FOR:
            • Loss of data or code
            • Business interruption
            • System crashes or failures
            • Security breaches or unauthorized access
            • Intellectual property infringement claims
            • Financial losses or damages
            • Personal injury or property damage
            
            6. INDEMNIFICATION
            You agree to indemnify and hold harmless the developers from any claims, damages, liabilities,
            costs, and expenses arising from:
            • Your use of the Software
            • Any violation of these Terms & Conditions
            • Your breach of any applicable laws or regulations
            • Your infringement of any third-party rights
            
            7. THIRD-PARTY SERVICES
            The Software integrates with and depends on several third-party services:
            • Google Gemini AI: Subject to Google's terms of service
            • Boost C++ Libraries: Under Boost Software License
            • OpenSSL: Under OpenSSL License
            • Various other open-source libraries
            
            8. INTELLECTUAL PROPERTY
            All intellectual property rights in the Software belong to the developers.
            You are granted a non-exclusive, non-transferable license to use the Software.
            
            9. DISCLAIMER OF WARRANTIES
            THE SOFTWARE IS PROVIDED "AS IS" AND "AS AVAILABLE" WITHOUT ANY WARRANTIES OF ANY KIND.
            
            10. ACKNOWLEDGMENT
            By clicking "I Accept" and continuing to use the Software, you acknowledge that:
            • You have read and understood these Terms & Conditions
            • You accept all risks associated with using the Software
            • You agree to hold the developers harmless from any liability
            • You understand that this is an experimental AI integration tool
            • You will use the Software responsibly and in compliance with all applicable laws
            """;
    }
    
    @Override
    public void dispose() {
        if (!accepted) {
            System.exit(0);
        }
        super.dispose();
    }
}