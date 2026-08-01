import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import "styles"

Window {
    id: root
    width: 920
    height: 700
    title: "NotY-Gemini-MCP"
    color: Theme.bgPrimary
    
    // Properties
    property bool vscodeInstalled: false
    property bool apiKeyValid: false
    property bool serverRunning: false
    property string apiKeyStatus: ""
    property string serverStatusText: "Not running"
    
    // Signals for C++ communication
    signal saveApiKey(string key)
    signal startServer()
    signal stopServer()
    signal browseVSCode()
    signal refreshVSCode()
    signal openExternalLink(string url)
    signal acceptTerms()
    signal declineTerms()
    
    // Main container
    Rectangle {
        id: container
        anchors.fill: parent
        anchors.margins: 20
        color: Theme.bgSecondary
        radius: Theme.radius
        border.color: Theme.borderColor
        border.width: 1
        
        // Header
        Column {
            id: header
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 24
            spacing: 8
            
            Text {
                text: "⚡ NotY-Gemini-MCP"
                font.pixelSize: 32
                font.weight: Font.Bold
                font.family: Theme.fontFamily
                color: Theme.textPrimary
                
                gradient: Gradient {
                    GradientStop { position: 0.0; color: Theme.accentPrimary }
                    GradientStop { position: 1.0; color: Theme.accentSecondary }
                }
            }
            
            Text {
                text: "AI-Powered Coding Assistant for VS Code"
                font.pixelSize: 15
                font.weight: Font.Light
                font.family: Theme.fontFamily
                color: Theme.textSecondary
                opacity: 0.8
            }
            
            Rectangle {
                id: statusBadge
                width: statusText.implicitWidth + 32
                height: 28
                radius: 14
                color: serverRunning ? Qt.rgba(72/255, 187/255, 120/255, 0.2) : Qt.rgba(245/255, 101/255, 101/255, 0.2)
                border.color: serverRunning ? Qt.rgba(72/255, 187/255, 120/255, 0.3) : Qt.rgba(245/255, 101/255, 101/255, 0.3)
                
                Text {
                    id: statusText
                    anchors.centerIn: parent
                    text: serverRunning ? "● Server Running" : "● Server Stopped"
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    font.family: Theme.fontFamily
                    color: serverRunning ? Theme.success : Theme.danger
                }
            }
        }
        
        // Content area
        Flickable {
            id: contentArea
            anchors.top: header.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 12
            contentHeight: contentColumn.height + 40
            boundsBehavior: Flickable.StopAtBounds
            
            Column {
                id: contentColumn
                width: parent.width - 24
                spacing: 16
                anchors.horizontalCenter: parent.horizontalCenter
                
                // VS Code Setup Section
                VSCodeSetup {
                    id: vscodeSetup
                    width: parent.width
                    onBrowseClicked: root.browseVSCode()
                    onRefreshClicked: root.refreshVSCode()
                    onDownloadClicked: root.openExternalLink("https://code.visualstudio.com/download?_exp_download=fb315fc982")
                }
                
                // API Key Setup Section
                ApiKeySetup {
                    id: apiKeySetup
                    width: parent.width
                    visible: vscodeSetup.installed
                    onSaveKey: root.saveApiKey(key)
                    onGetKey: root.openExternalLink("https://aistudio.google.com/api-keys")
                }
                
                // Server Control Section
                ServerControl {
                    id: serverControl
                    width: parent.width
                    visible: apiKeySetup.valid
                    serverRunning: root.serverRunning
                    onStartServer: root.startServer()
                    onStopServer: root.stopServer()
                }
                
                // Footer
                Rectangle {
                    width: parent.width
                    height: 70
                    color: "transparent"
                    
                    Column {
                        anchors.centerIn: parent
                        spacing: 4
                        
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "NotY-Gemini-MCP v1.0.0 | Made with ❤️ by NotY215/Fliczo"
                            font.pixelSize: 12
                            font.family: Theme.fontFamily
                            color: Theme.textMuted
                        }
                        
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "Click here to view Terms & Conditions"
                            font.pixelSize: 11
                            font.family: Theme.fontFamily
                            color: Theme.accentPrimary
                            underline: true
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: termsDialog.open()
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Terms Dialog
    TermsDialog {
        id: termsDialog
        onAccepted: root.acceptTerms()
        onDeclined: root.declineTerms()
    }
    
    // Toast Notification
    ToastNotification {
        id: toast
        visible: false
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter
    }
    
    // Functions
    function showToast(message, type) {
        toast.message = message
        toast.type = type
        toast.visible = true
        toast.startAnimation()
    }
    
    function updateVSCodeStatus(installed, path) {
        vscodeSetup.installed = installed
        vscodeSetup.installedPath = path
        if (!installed) {
            showToast("VS Code not found. Please install or browse for VS Code.", "warning")
        } else {
            showToast("VS Code found at: " + path, "success")
        }
    }
    
    function updateApiKeyStatus(valid) {
        apiKeySetup.valid = valid
        if (valid) {
            showToast("API key verified successfully!", "success")
        } else {
            showToast("Invalid API key. Please check and try again.", "error")
        }
    }
    
    function updateServerStatus(running) {
        serverRunning = running
        serverControl.serverRunning = running
        if (running) {
            showToast("Server started successfully!", "success")
        } else {
            showToast("Server stopped.", "info")
        }
    }
}