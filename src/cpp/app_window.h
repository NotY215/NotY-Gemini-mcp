#pragma once
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QStackedWidget>
#include <QProgressBar>
#include <QTimer>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QFileDialog>
#include <functional>
#include <string>
#include <memory>

class AppWindow : public QMainWindow {
    Q_OBJECT

private:
    // UI Components
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    QStackedWidget* stackedWidget;

    // Header
    QLabel* titleLabel;
    QLabel* subtitleLabel;
    QLabel* statusBadge;

    // Step 1: VS Code Setup
    QWidget* vscodeWidget;
    QLabel* vscodeStatus;
    QLabel* vscodePathLabel;
    QPushButton* browseVSCodeBtn;
    QPushButton* refreshVSCodeBtn;
    QPushButton* downloadVSCodeBtn;

    // Step 2: API Key Setup
    QWidget* apiKeyWidget;
    QLineEdit* apiKeyInput;
    QPushButton* saveApiKeyBtn;
    QPushButton* getApiKeyBtn;
    QLabel* apiKeyStatusLabel;
    QCheckBox* showKeyCheckBox;

    // Step 3: Server Control
    QWidget* serverWidget;
    QPushButton* startServerBtn;
    QPushButton* stopServerBtn;
    QLabel* serverStatusLabel;
    QTextEdit* logArea;

    // Footer
    QLabel* footerLabel;
    QPushButton* termsLink;

    // Tray Icon
    std::unique_ptr<QSystemTrayIcon> trayIcon;
    std::unique_ptr<QMenu> trayMenu;

    // Callbacks
    std::function<void()> onCloseCallback;
    std::function<void(const std::string&)> onWebMessageCallback;

    // State
    bool vscodeInstalled;
    bool apiKeyValid;
    bool serverRunning;
    QString vscodePath;

    void setupUI();
    void setupTrayIcon();
    void updateUI();

    // Logging
    void addLog(const QString& message, const QString& type = "info");

public:
    AppWindow(QWidget* parent = nullptr);
    ~AppWindow();

    bool create();
    void show();
    void hide();
    void close();
    void setTitle(const std::string& title);
    void setIcon(const std::string& path);

    void setOnClose(std::function<void()> callback);
    void setOnWebMessage(std::function<void(const std::string&)> callback);
    void sendToWeb(const std::string& message);

    // Public slots for QML/JS communication
    void updateVSCodeStatus(bool installed, const QString& path);
    void updateApiKeyStatus(bool valid);
    void updateServerStatus(bool running);
    void showToast(const QString& message, const QString& type = "info");

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onBrowseVSCode();
    void onRefreshVSCode();
    void onDownloadVSCode();
    void onSaveApiKey();
    void onGetApiKey();
    void onStartServer();
    void onStopServer();
    void onTermsClicked();
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onExitAction();
    void onShowKeyToggled(bool checked);
};