#include "app_window.h"
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickItem>
#include <QFile>
#include <QDir>
#include <QIcon>
#include <QStandardPaths>

AppWindow::AppWindow(int w, int h) : isVisible(false) {
    view = std::make_unique<QQuickView>();
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    view->setWidth(w);
    view->setHeight(h);
    
    // Connect QML signals
    QObject::connect(view->rootContext(), &QQmlContext::signalEmitted, 
                     this, &AppWindow::handleQmlMessage);
}

AppWindow::~AppWindow() {
    if (view) {
        view->close();
    }
}

bool AppWindow::create() {
    // Set up QML context
    view->rootContext()->setContextProperty("appWindow", this);
    
    // Load QML from resources
    view->setSource(QUrl("qrc:/ui/main.qml"));
    
    if (view->status() != QQuickView::Ready) {
        return false;
    }
    
    // Set window flags
    view->setFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    
    return true;
}

void AppWindow::show() {
    if (view) {
        view->show();
        view->raise();
        view->requestActivate();
        isVisible = true;
    }
}

void AppWindow::hide() {
    if (view) {
        view->hide();
        isVisible = false;
    }
}

void AppWindow::close() {
    if (view) {
        view->close();
    }
}

void AppWindow::setTitle(const std::string& title) {
    if (view) {
        view->setTitle(QString::fromStdString(title));
    }
}

void AppWindow::setIcon(const std::string& path) {
    if (view) {
        QIcon icon(QString::fromStdString(path));
        view->setIcon(icon);
    }
}

void AppWindow::setOnClose(std::function<void()> callback) {
    onCloseCallback = callback;
}

void AppWindow::setOnWebMessage(std::function<void(const std::string&)> callback) {
    onWebMessageCallback = callback;
}

void AppWindow::sendToWeb(const std::string& message) {
    // Send message to QML
    if (view && view->rootObject()) {
        QMetaObject::invokeMethod(view->rootObject(), "handleMessage",
            Q_ARG(QVariant, QString::fromStdString(message)));
    }
}

void AppWindow::handleQmlMessage(const QString& message) {
    if (onWebMessageCallback) {
        onWebMessageCallback(message.toStdString());
    }
}