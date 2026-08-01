import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "styles"

Rectangle {
    id: dialog
    visible: false
    anchors.fill: parent
    color: Qt.rgba(0, 0, 0, 0.7)
    z: 100
    
    signal accepted()
    signal declined()
    
    Rectangle {
        width: 700
        height: 550
        anchors.centerIn: parent
        color: Theme.bgSecondary
        radius: Theme.radius
        border.color: Theme.borderColor
        border.width: 1
        
        Column {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 12
            
            Text {
                text: "📜 Terms & Conditions"
                font.pixelSize: 26
                font.weight: Font.Bold
                font.family: Theme.fontFamily
                color: Theme.textPrimary
            }
            
            Text {
                text: "NotY-Gemini-MCP v1.0.0"
                font.pixelSize: 13
                font.family: Theme.fontFamily
                color: Theme.textMuted
            }
            
            Rectangle {
                width: parent.width
                height: 300
                color: Qt.rgba(0, 0, 0, 0.3)
                radius: 10
                border.color: Theme.borderColor
                border.width: 1
                
                Flickable {
                    anchors.fill: parent
                    anchors.margins: 12
                    contentHeight: termsText.height + 20
                    clip: true
                    
                    Text {
                        id: termsText
                        width: parent.width - 8
                        text: `
⚠️ DISCLAIMER OF LIABILITY
The developers and contributors of NotY-Gemini-MCP are NOT responsible for any data breaches, security incidents, or unauthorized access to your systems. This software is provided "AS IS" and you use it entirely at your own risk.

1. ACCEPTANCE OF TERMS
By installing, downloading, or using NotY-Gemini-MCP, you agree to comply with and be bound by these Terms & Conditions. If you do not agree to these terms, you must immediately uninstall and discontinue use of the Software.

2. DATA HANDLING AND PRIVACY
Your Gemini API key is encrypted using AES-256 encryption and stored locally. When you use the Software to interact with Google's Gemini AI, the following data may be transmitted to Google's servers:
- Your Gemini API key (for authentication)
- Code snippets and project files you choose to analyze
- Natural language queries and conversations
- Error logs and debugging information

3. SECURITY RISKS
Using AI-powered coding assistants involves inherent security risks, including but not limited to:
- Data Exposure: Code snippets may be temporarily stored by Google's servers
- Intellectual Property: Sharing proprietary code with AI services may have implications
- API Key Compromise: If your API key is exposed, unauthorized users could access your Gemini AI quota

4. USER OBLIGATIONS
By using the Software, you agree to:
- Keep your Gemini API key secure
- Review all AI-generated code suggestions before implementing them
- Not use the Software for illegal or malicious purposes
- Ensure you have the rights to analyze any code you submit
- Not upload sensitive credentials, passwords, or private keys

5. COMPLETE DISCLAIMER OF LIABILITY
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THE DEVELOPERS SHALL NOT BE LIABLE FOR:
- Loss of data or code
- Business interruption
- System crashes or failures
- Security breaches or unauthorized access
- Intellectual property infringement claims
- Financial losses or damages

6. INDEMNIFICATION
You agree to indemnify and hold harmless the developers from any claims, damages, liabilities, costs, and expenses arising from your use of the Software.

7. GOVERNING LAW
These Terms shall be governed by and construed in accordance with the laws of the jurisdiction where the developers are located.

8. DISCLAIMER OF WARRANTIES
THE SOFTWARE IS PROVIDED "AS IS" AND "AS AVAILABLE" WITHOUT ANY WARRANTIES OF ANY KIND.

9. ACKNOWLEDGMENT
By clicking "I Accept" and continuing to use the Software, you acknowledge that you have read and understood these Terms & Conditions, accept all risks, and agree to hold the developers harmless.
                        `
                        font.pixelSize: 13
                        font.family: Theme.fontFamily
                        color: Theme.textSecondary
                        wrapMode: Text.WordWrap
                        lineHeight: 1.5
                    }
                }
            }
            
            Row {
                width: parent.width
                spacing: 12
                anchors.horizontalCenter: parent.horizontalCenter
                
                Button {
                    text: "✅ I Accept"
                    width: 150
                    height: 40
                    font.pixelSize: 14
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
                    onClicked: {
                        dialog.visible = false
                        dialog.accepted()
                    }
                }
                
                Button {
                    text: "❌ I Decline"
                    width: 150
                    height: 40
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    font.family: Theme.fontFamily
                    background: Rectangle {
                        color: parent.hovered ? Qt.darker(Theme.danger, 1.2) : Theme.danger
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
                        dialog.visible = false
                        dialog.declined()
                    }
                }
            }
        }
    }
}