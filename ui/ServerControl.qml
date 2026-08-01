import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "styles"

Rectangle {
    id: root
    color: Theme.bgCard
    radius: Theme.radius
    border.color: Theme.borderColor
    border.width: 1
    property bool serverRunning: false
    
    signal startServer()
    signal stopServer()
    
    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12
        
        Row {
            width: parent.width
            spacing: 12
            
            Text {
                text: "🚀 Step 3: MCP Server"
                font.pixelSize: 17
                font.weight: Font.Medium
                font.family: Theme.fontFamily
                color: Theme.textPrimary
            }
            
            Rectangle {
                width: 28
                height: 28
                radius: 14
                gradient: Theme.accentGradient
                
                Text {
                    anchors.centerIn: parent
                    text: "3"
                    font.pixelSize: 13
                    font.weight: Font.Bold
                    color: "white"
                }
            }
        }
        
        Row {
            width: parent.width
            spacing: 10
            
            Button {
                id: startBtn
                text: serverRunning ? "▶ Running" : "▶ Start Server"
                font.pixelSize: 13
                font.weight: Font.Medium
                font.family: Theme.fontFamily
                enabled: !serverRunning
                background: Rectangle {
                    color: startBtn.hovered && !startBtn.enabled ? Theme.bgCardHover : Theme.accentPrimary
                    radius: 10
                    gradient: startBtn.enabled ? Theme.accentGradient : undefined
                    opacity: startBtn.enabled ? 1.0 : 0.5
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: root.startServer()
            }
            
            Button {
                id: stopBtn
                text: "⏹ Stop Server"
                font.pixelSize: 13
                font.weight: Font.Medium
                font.family: Theme.fontFamily
                enabled: serverRunning
                background: Rectangle {
                    color: stopBtn.hovered && stopBtn.enabled ? Qt.darker(Theme.danger, 1.2) : Theme.danger
                    radius: 10
                    opacity: stopBtn.enabled ? 1.0 : 0.5
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: root.stopServer()
            }
        }
        
        Row {
            width: parent.width
            spacing: 4
            
            Text {
                text: "Status:"
                font.pixelSize: 13
                font.family: Theme.fontFamily
                color: Theme.textSecondary
            }
            
            Text {
                text: serverRunning ? "🟢 Running" : "🔴 Stopped"
                font.pixelSize: 13
                font.family: Theme.fontFamily
                color: serverRunning ? Theme.success : Theme.danger
            }
        }
        
        Text {
            text: "Server runs in background. Close window to minimize to tray."
            font.pixelSize: 12
            font.family: Theme.fontFamily
            color: Theme.textMuted
        }
        
        // Log Area
        Rectangle {
            width: parent.width
            height: 120
            color: Qt.rgba(0, 0, 0, 0.4)
            radius: 10
            border.color: Theme.borderColor
            border.width: 1
            
            Flickable {
                anchors.fill: parent
                anchors.margins: 8
                contentHeight: logColumn.height
                clip: true
                
                Column {
                    id: logColumn
                    width: parent.width
                    spacing: 2
                    
                    LogEntry { text: "🚀 Application initialized" }
                    LogEntry { text: "📋 Ready to start server" }
                }
            }
        }
    }
}

// Helper component for log entries
component LogEntry: Text {
    text: text
    font.pixelSize: 12
    font.family: "Consolas, monospace"
    color: {
        if (text.indexOf("✅") !== -1 || text.indexOf("🚀") !== -1) return Theme.success
        if (text.indexOf("❌") !== -1 || text.indexOf("Error") !== -1) return Theme.danger
        return Theme.textMuted
    }
    wrapMode: Text.WordWrap
    width: parent.width
}