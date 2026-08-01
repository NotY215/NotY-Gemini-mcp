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
    property alias installed: statusIndicator.installed
    property string installedPath: ""
    
    signal browseClicked()
    signal refreshClicked()
    signal downloadClicked()
    
    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12
        
        Row {
            width: parent.width
            spacing: 12
            
            Text {
                text: "📦 Step 1: VS Code Setup"
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
                    text: "1"
                    font.pixelSize: 13
                    font.weight: Font.Bold
                    color: "white"
                }
            }
        }
        
        Rectangle {
            width: parent.width
            height: 50
            color: Qt.rgba(0, 0, 0, 0.2)
            radius: 10
            border.color: Theme.borderColor
            border.width: 1
            
            Row {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12
                
                Rectangle {
                    width: 10
                    height: 10
                    radius: 5
                    color: installed ? Theme.success : Theme.danger
                    opacity: 0.8
                }
                
                Text {
                    text: installed ? "✅ VS Code is installed" : "❌ VS Code is not installed"
                    font.pixelSize: 13
                    font.family: Theme.fontFamily
                    color: Theme.textSecondary
                }
            }
            
            Text {
                anchors.right: parent.right
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                text: installedPath ? "Path: " + installedPath : ""
                font.pixelSize: 11
                font.family: Theme.fontFamily
                color: Theme.textMuted
                visible: installed
                elide: Text.ElideLeft
                width: parent.width * 0.4
            }
        }
        
        Row {
            width: parent.width
            spacing: 10
            
            Button {
                text: "📂 Browse"
                font.pixelSize: 13
                font.weight: Font.Medium
                font.family: Theme.fontFamily
                background: Rectangle {
                    color: parent.hovered ? Theme.bgCardHover : Theme.accentPrimary
                    radius: 10
                    gradient: Theme.accentGradient
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: root.browseClicked()
            }
            
            Button {
                text: "🔄 Refresh"
                font.pixelSize: 13
                font.weight: Font.Medium
                font.family: Theme.fontFamily
                background: Rectangle {
                    color: parent.hovered ? Theme.bgCardHover : "transparent"
                    radius: 10
                    border.color: Theme.borderColor
                    border.width: 1
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.textSecondary
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: root.refreshClicked()
            }
            
            Button {
                text: "⬇ Download VS Code"
                font.pixelSize: 13
                font.weight: Font.Medium
                font.family: Theme.fontFamily
                background: Rectangle {
                    color: parent.hovered ? Theme.bgCardHover : "transparent"
                    radius: 10
                }
                contentItem: Text {
                    text: parent.text
                    color: Theme.accentPrimary
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.underline: true
                }
                onClicked: root.downloadClicked()
            }
        }
    }
}