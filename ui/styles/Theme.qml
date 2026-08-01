pragma Singleton
import QtQuick 2.15

QtObject {
    // Windows 11 Dark Theme Colors
    property color bgPrimary: "#1a1a2e"
    property color bgSecondary: "#16213e"
    property color bgCard: "#0f3460"
    property color bgCardHover: "#1a3a6a"
    property color textPrimary: "#e0e0e0"
    property color textSecondary: "#a0a0a0"
    property color textMuted: "#6b6b6b"
    property color borderColor: "#2a2a4a"
    property color accentPrimary: "#667eea"
    property color accentSecondary: "#764ba2"
    property color success: "#48bb78"
    property color danger: "#f56565"
    property color warning: "#f6ad55"
    property color info: "#4299e1"
    
    // Gradients
    property Gradient accentGradient: Gradient {
        GradientStop { position: 0.0; color: "#667eea" }
        GradientStop { position: 1.0; color: "#764ba2" }
    }
    
    // Sizes
    property int radius: 16
    property int spacing: 12
    
    // Fonts
    property string fontFamily: "Segoe UI, -apple-system, BlinkMacSystemFont, Roboto, sans-serif"
}