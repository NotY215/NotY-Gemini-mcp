#pragma once
#include <QObject>
#include <QQuickView>
#include <QQmlContext>
#include <QString>
#include <functional>
#include <memory>

class AppWindow : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<QQuickView> view;
    bool isVisible;

    std::function<void()> onCloseCallback;
    std::function<void(const std::string&)> onWebMessageCallback;

public:
    AppWindow(int w, int h);
    ~AppWindow();

    bool create();
    void show();
    void hide();
    void close();
    void setTitle(const std::string& title);
    void setIcon(const std::string& path);
    QWindow* getHandle() const { return view ? view->window() : nullptr; }

    void setOnClose(std::function<void()> callback);
    void setOnWebMessage(std::function<void(const std::string&)> callback);
    void sendToWeb(const std::string& message);

public slots:
    void handleQmlMessage(const QString& message);

signals:
    void qmlMessage(const QString& message);
};