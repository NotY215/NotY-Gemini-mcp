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
    property alias valid: statusText.visible && statusText.text === "✅ Verified"
    property alias key: apiKeyInput.text
    
    signal saveKey(string key)
    signal getKey()
    
    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12
        
        Row {
            width: parent.width
            spacing: 12
            
            Text {
                text: "🔑 Step 2: Gemini API Key"
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
                    text: "2"
                    font.pixelSize: 13
                    font.weight: Font.Bold
                    color: "white"
                }
            }
        }
        
        Column {
            width: parent.width
            spacing: 6
            
            Text {
                text: "Enter your Gemini API Key"
                font.pixelSize: 14
                font.weight: Font.Medium
                font.family: Theme.fontFamily
                color: Theme.textPrimary
            }
            
            Row {
                width: parent.width
                spacing: 8
                
                Rectangle {
                    width: parent.width - 140
                    height: 44
                    radius: 10
                    border.color: Theme.borderColor
                    border.width: 1
                    color: Qt.rgba(0, 0, 0, 0.3)
                    
                    TextField {
                        id: apiKeyInput
                        anchors.fill: parent
                        anchors.margins: 4
                        placeholderText: "Paste your API key here..."
                        font.pixelSize: 14
                        font.family: Theme.fontFamily
                        color: Theme.textPrimary
                        echoMode: TextField.Password
                        background: Rectangle {
                            color: "transparent"
                            radius: 10
                        }
                        selectByMouse: true
                    }
                }
                
                Button {
                    id: saveBtn
                    width: 120
                    height: 44
                    text: "💾 Save & Verify"
                    font.pixelSize: 13
                    font.weight: Font.Medium
                    font.family: Theme.fontFamily
                    background: Rectangle {
                        color: saveBtn.hovered ? Qt.darker(Theme.success, 1.2) : Theme.success
                        radius: 10
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font: parent.font
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        if (apiKeyInput.text.trim().length > 0) {
                            root.saveKey(apiKeyInput.text.trim())
                        }
                    }
                }
            }
            
            Text {
                id: statusText
                text: ""
                font.pixelSize: 13
                font.family: Theme.fontFamily
                color: valid ? Theme.success : Theme.danger
                visible: text.length > 0
            }
            
            Text {
                text: "🔒 Your API key is encrypted and stored locally"
                font.pixelSize: 12
                font.family: Theme.fontFamily
                color: Theme.textMuted
            }
        }
        
        Row {
            width: parent.width
            spacing: 10
            
            Button {
                text: "🔑 Get API Key"
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
                onClicked: root.getKey()
            }
        }
    }
}