import QtQuick 2.15
import QtQuick.Controls 2.15
import "styles"

Rectangle {
    id: root
    width: 400
    height: 50
    radius: 12
    color: {
        if (type === "success") return Qt.rgba(72/255, 187/255, 120/255, 0.9)
        if (type === "error") return Qt.rgba(245/255, 101/255, 101/255, 0.9)
        if (type === "warning") return Qt.rgba(246/255, 173/255, 85/255, 0.9)
        return Qt.rgba(66/255, 153/255, 225/255, 0.9)
    }
    border.color: {
        if (type === "success") return Qt.rgba(72/255, 187/255, 120/255, 0.3)
        if (type === "error") return Qt.rgba(245/255, 101/255, 101/255, 0.3)
        if (type === "warning") return Qt.rgba(246/255, 173/255, 85/255, 0.3)
        return Qt.rgba(66/255, 153/255, 225/255, 0.3)
    }
    border.width: 1
    property string message: ""
    property string type: "info"
    opacity: 0
    y: parent.height + 20
    
    Text {
        anchors.centerIn: parent
        text: root.message
        font.pixelSize: 14
        font.family: Theme.fontFamily
        color: "white"
        font.weight: Font.Medium
    }
    
    function startAnimation() {
        opacity = 0
        y = parent.height + 20
        showAnimation.start()
    }
    
    NumberAnimation {
        id: showAnimation
        target: root
        property: "opacity"
        from: 0
        to: 1
        duration: 300
        onFinished: {
            hideTimer.start()
        }
    }
    
    NumberAnimation {
        id: hideAnimation
        target: root
        property: "opacity"
        from: 1
        to: 0
        duration: 300
        onFinished: {
            root.visible = false
        }
    }
    
    Timer {
        id: hideTimer
        interval: 3000
        onTriggered: {
            hideAnimation.start()
        }
    }
}