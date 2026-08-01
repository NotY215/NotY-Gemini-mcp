#include "app_window.h"
#include <QApplication>
#include <QDesktopServices>
#include <QStyle>
#include <QUrl>
#include <QPalette>
#include <QScrollArea>
#include <QFrame>
#include <QIcon>
#include <QFontDatabase>
#include <QDateTime>
#include <QThread>

// Windows 11 Dark Theme Colors
static const QString DARK_BG_PRIMARY = "#1a1a2e";
static const QString DARK_BG_SECONDARY = "#16213e";
static const QString DARK_BG_CARD = "#0f3460";
static const QString DARK_TEXT_PRIMARY = "#e0e0e0";
static const QString DARK_TEXT_SECONDARY = "#a0a0a0";
static const QString DARK_TEXT_MUTED = "#6b6b6b";
static const QString DARK_BORDER = "#2a2a4a";
static const QString DARK_ACCENT = "#667eea";
static const QString DARK_SUCCESS = "#48bb78";
static const QString DARK_DANGER = "#f56565";
static const QString DARK_WARNING = "#f6ad55";

// Helper function to set dark theme
static void applyDarkTheme(QWidget* widget) {
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(DARK_BG_SECONDARY));
    darkPalette.setColor(QPalette::WindowText, QColor(DARK_TEXT_PRIMARY));
    darkPalette.setColor(QPalette::Base, QColor(DARK_BG_PRIMARY));
    darkPalette.setColor(QPalette::AlternateBase, QColor(DARK_BG_CARD));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(DARK_BG_SECONDARY));
    darkPalette.setColor(QPalette::ToolTipText, QColor(DARK_TEXT_PRIMARY));
    darkPalette.setColor(QPalette::Text, QColor(DARK_TEXT_PRIMARY));
    darkPalette.setColor(QPalette::Button, QColor(DARK_BG_CARD));
    darkPalette.setColor(QPalette::ButtonText, QColor(DARK_TEXT_PRIMARY));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(DARK_ACCENT));
    darkPalette.setColor(QPalette::Highlight, QColor(DARK_ACCENT));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    widget->setPalette(darkPalette);
}

AppWindow::AppWindow(QWidget* parent)
    : QMainWindow(parent),
    vscodeInstalled(false),
    apiKeyValid(false),
    serverRunning(false) {
    setupUI();
    setupTrayIcon();
    applyDarkTheme(this);
    setWindowTitle("NotY-Gemini-MCP");
    setMinimumSize(900, 650);
}

AppWindow::~AppWindow() {
    if (trayIcon) {
        trayIcon->hide();
    }
}

void AppWindow::setupUI() {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    // Header
    QWidget* headerWidget = new QWidget();
    QVBoxLayout* headerLayout = new QVBoxLayout(headerWidget);
    headerLayout->setSpacing(4);

    titleLabel = new QLabel("⚡ NotY-Gemini-MCP");
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: " + DARK_TEXT_PRIMARY + ";");
    headerLayout->addWidget(titleLabel);

    subtitleLabel = new QLabel("AI-Powered Coding Assistant for VS Code");
    subtitleLabel->setStyleSheet("font-size: 14px; color: " + DARK_TEXT_SECONDARY + ";");
    headerLayout->addWidget(subtitleLabel);

    statusBadge = new QLabel("● Server Stopped");
    statusBadge->setStyleSheet("font-size: 12px; font-weight: 500; padding: 4px 16px; "
        "border-radius: 12px; background-color: rgba(245, 101, 101, 0.2); "
        "color: " + DARK_DANGER + ";");
    headerLayout->addWidget(statusBadge);

    mainLayout->addWidget(headerWidget);

    // Content area with scroll
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    QWidget* contentWidget = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(16);

    // Step 1: VS Code Setup
    vscodeWidget = new QWidget();
    vscodeWidget->setStyleSheet("background-color: " + DARK_BG_CARD + "; border-radius: 12px; padding: 16px;");
    QVBoxLayout* vscodeLayout = new QVBoxLayout(vscodeWidget);

    QHBoxLayout* vscodeHeader = new QHBoxLayout();
    QLabel* vscodeTitle = new QLabel("📦 Step 1: VS Code Setup");
    vscodeTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: " + DARK_TEXT_PRIMARY + ";");
    vscodeHeader->addWidget(vscodeTitle);

    QLabel* stepNumber1 = new QLabel("1");
    stepNumber1->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #667eea, stop:1 #764ba2); color: white; "
        "border-radius: 14px; padding: 4px 10px; font-weight: bold;");
    vscodeHeader->addWidget(stepNumber1);
    vscodeHeader->addStretch();
    vscodeLayout->addLayout(vscodeHeader);

    QHBoxLayout* vscodeStatusLayout = new QHBoxLayout();
    vscodeStatusLayout->setSpacing(12);
    vscodeStatus = new QLabel("❌ Checking VS Code installation...");
    vscodeStatus->setStyleSheet("color: " + DARK_TEXT_SECONDARY + "; padding: 8px 12px; "
        "background-color: rgba(0,0,0,0.2); border-radius: 8px;");
    vscodeStatusLayout->addWidget(vscodeStatus);

    vscodePathLabel = new QLabel();
    vscodePathLabel->setStyleSheet("color: " + DARK_TEXT_MUTED + "; font-size: 11px;");
    vscodePathLabel->setVisible(false);
    vscodeStatusLayout->addWidget(vscodePathLabel);
    vscodeLayout->addLayout(vscodeStatusLayout);

    QHBoxLayout* vscodeButtons = new QHBoxLayout();
    vscodeButtons->setSpacing(10);

    browseVSCodeBtn = new QPushButton("📂 Browse");
    browseVSCodeBtn->setStyleSheet("QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #667eea, stop:1 #764ba2); color: white; border: none; "
        "padding: 8px 20px; border-radius: 8px; font-weight: 600; } "
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #7b8ff5, stop:1 #8b5ab5); }");
    connect(browseVSCodeBtn, &QPushButton::clicked, this, &AppWindow::onBrowseVSCode);
    vscodeButtons->addWidget(browseVSCodeBtn);

    refreshVSCodeBtn = new QPushButton("🔄 Refresh");
    refreshVSCodeBtn->setStyleSheet("QPushButton { background: transparent; color: " + DARK_TEXT_SECONDARY +
        "; border: 1px solid " + DARK_BORDER + "; padding: 8px 20px; border-radius: 8px; "
        "font-weight: 600; } QPushButton:hover { background: rgba(255,255,255,0.05); }");
    connect(refreshVSCodeBtn, &QPushButton::clicked, this, &AppWindow::onRefreshVSCode);
    vscodeButtons->addWidget(refreshVSCodeBtn);

    downloadVSCodeBtn = new QPushButton("⬇ Download VS Code");
    downloadVSCodeBtn->setStyleSheet("QPushButton { background: transparent; color: " + DARK_ACCENT +
        "; border: none; padding: 8px 16px; border-radius: 8px; "
        "font-weight: 600; text-decoration: underline; } "
        "QPushButton:hover { background: rgba(102, 126, 234, 0.1); }");
    connect(downloadVSCodeBtn, &QPushButton::clicked, this, &AppWindow::onDownloadVSCode);
    vscodeButtons->addWidget(downloadVSCodeBtn);
    vscodeButtons->addStretch();
    vscodeLayout->addLayout(vscodeButtons);

    contentLayout->addWidget(vscodeWidget);

    // Step 2: API Key Setup
    apiKeyWidget = new QWidget();
    apiKeyWidget->setStyleSheet("background-color: " + DARK_BG_CARD + "; border-radius: 12px; padding: 16px;");
    apiKeyWidget->setVisible(false);
    QVBoxLayout* apiKeyLayout = new QVBoxLayout(apiKeyWidget);

    QHBoxLayout* apiKeyHeader = new QHBoxLayout();
    QLabel* apiKeyTitle = new QLabel("🔑 Step 2: Gemini API Key");
    apiKeyTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: " + DARK_TEXT_PRIMARY + ";");
    apiKeyHeader->addWidget(apiKeyTitle);

    QLabel* stepNumber2 = new QLabel("2");
    stepNumber2->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #667eea, stop:1 #764ba2); color: white; "
        "border-radius: 14px; padding: 4px 10px; font-weight: bold;");
    apiKeyHeader->addWidget(stepNumber2);
    apiKeyHeader->addStretch();
    apiKeyLayout->addLayout(apiKeyHeader);

    QLabel* apiKeyLabel = new QLabel("Enter your Gemini API Key");
    apiKeyLabel->setStyleSheet("color: " + DARK_TEXT_PRIMARY + "; font-weight: 500;");
    apiKeyLayout->addWidget(apiKeyLabel);

    QHBoxLayout* apiKeyInputLayout = new QHBoxLayout();
    apiKeyInput = new QLineEdit();
    apiKeyInput->setPlaceholderText("Paste your API key here...");
    apiKeyInput->setEchoMode(QLineEdit::Password);
    apiKeyInput->setStyleSheet("QLineEdit { background-color: rgba(0,0,0,0.3); border: 1px solid " + DARK_BORDER +
        "; border-radius: 8px; padding: 8px 12px; color: " + DARK_TEXT_PRIMARY +
        "; } QLineEdit:focus { border-color: " + DARK_ACCENT + "; }");
    apiKeyInputLayout->addWidget(apiKeyInput);

    showKeyCheckBox = new QCheckBox("👁");
    showKeyCheckBox->setStyleSheet("QCheckBox { color: " + DARK_TEXT_MUTED + "; }");
    connect(showKeyCheckBox, &QCheckBox::toggled, this, &AppWindow::onShowKeyToggled);
    apiKeyInputLayout->addWidget(showKeyCheckBox);
    apiKeyLayout->addLayout(apiKeyInputLayout);

    QHBoxLayout* apiKeyButtons = new QHBoxLayout();
    apiKeyButtons->setSpacing(10);

    saveApiKeyBtn = new QPushButton("💾 Save & Verify");
    saveApiKeyBtn->setStyleSheet("QPushButton { background-color: " + DARK_SUCCESS + "; color: white; "
        "border: none; padding: 8px 20px; border-radius: 8px; font-weight: 600; } "
        "QPushButton:hover { background-color: #38a169; } "
        "QPushButton:disabled { opacity: 0.5; }");
    connect(saveApiKeyBtn, &QPushButton::clicked, this, &AppWindow::onSaveApiKey);
    apiKeyButtons->addWidget(saveApiKeyBtn);

    getApiKeyBtn = new QPushButton("🔑 Get API Key");
    getApiKeyBtn->setStyleSheet("QPushButton { background: transparent; color: " + DARK_ACCENT +
        "; border: none; padding: 8px 16px; border-radius: 8px; "
        "font-weight: 600; text-decoration: underline; } "
        "QPushButton:hover { background: rgba(102, 126, 234, 0.1); }");
    connect(getApiKeyBtn, &QPushButton::clicked, this, &AppWindow::onGetApiKey);
    apiKeyButtons->addWidget(getApiKeyBtn);
    apiKeyButtons->addStretch();
    apiKeyLayout->addLayout(apiKeyButtons);

    apiKeyStatusLabel = new QLabel();
    apiKeyStatusLabel->setStyleSheet("color: " + DARK_TEXT_MUTED + "; font-size: 13px;");
    apiKeyStatusLabel->setVisible(false);
    apiKeyLayout->addWidget(apiKeyStatusLabel);

    QLabel* apiKeyHint = new QLabel("🔒 Your API key is encrypted and stored locally");
    apiKeyHint->setStyleSheet("color: " + DARK_TEXT_MUTED + "; font-size: 12px;");
    apiKeyLayout->addWidget(apiKeyHint);

    contentLayout->addWidget(apiKeyWidget);

    // Step 3: Server Control
    serverWidget = new QWidget();
    serverWidget->setStyleSheet("background-color: " + DARK_BG_CARD + "; border-radius: 12px; padding: 16px;");
    serverWidget->setVisible(false);
    QVBoxLayout* serverLayout = new QVBoxLayout(serverWidget);

    QHBoxLayout* serverHeader = new QHBoxLayout();
    QLabel* serverTitle = new QLabel("🚀 Step 3: MCP Server");
    serverTitle->setStyleSheet("font-size: 16px; font-weight: 600; color: " + DARK_TEXT_PRIMARY + ";");
    serverHeader->addWidget(serverTitle);

    QLabel* stepNumber3 = new QLabel("3");
    stepNumber3->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #667eea, stop:1 #764ba2); color: white; "
        "border-radius: 14px; padding: 4px 10px; font-weight: bold;");
    serverHeader->addWidget(stepNumber3);
    serverHeader->addStretch();
    serverLayout->addLayout(serverHeader);

    QHBoxLayout* serverButtons = new QHBoxLayout();
    serverButtons->setSpacing(10);

    startServerBtn = new QPushButton("▶ Start Server");
    startServerBtn->setStyleSheet("QPushButton { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #667eea, stop:1 #764ba2); color: white; border: none; "
        "padding: 8px 20px; border-radius: 8px; font-weight: 600; } "
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "stop:0 #7b8ff5, stop:1 #8b5ab5); } "
        "QPushButton:disabled { opacity: 0.5; }");
    connect(startServerBtn, &QPushButton::clicked, this, &AppWindow::onStartServer);
    serverButtons->addWidget(startServerBtn);

    stopServerBtn = new QPushButton("⏹ Stop Server");
    stopServerBtn->setStyleSheet("QPushButton { background-color: " + DARK_DANGER + "; color: white; "
        "border: none; padding: 8px 20px; border-radius: 8px; font-weight: 600; } "
        "QPushButton:hover { background-color: #e53e3e; } "
        "QPushButton:disabled { opacity: 0.5; }");
    stopServerBtn->setEnabled(false);
    connect(stopServerBtn, &QPushButton::clicked, this, &AppWindow::onStopServer);
    serverButtons->addWidget(stopServerBtn);
    serverButtons->addStretch();
    serverLayout->addLayout(serverButtons);

    serverStatusLabel = new QLabel("Status: 🔴 Stopped");
    serverStatusLabel->setStyleSheet("color: " + DARK_DANGER + "; font-weight: 500;");
    serverLayout->addWidget(serverStatusLabel);

    QLabel* serverHint = new QLabel("Server runs in background. Close window to minimize to tray.");
    serverHint->setStyleSheet("color: " + DARK_TEXT_MUTED + "; font-size: 12px;");
    serverLayout->addWidget(serverHint);

    // Log Area
    logArea = new QTextEdit();
    logArea->setReadOnly(true);
    logArea->setStyleSheet("QTextEdit { background-color: rgba(0,0,0,0.4); color: " + DARK_TEXT_SECONDARY +
        "; border: 1px solid " + DARK_BORDER + "; border-radius: 8px; "
        "font-family: Consolas, monospace; font-size: 12px; padding: 8px; }");
    logArea->setMaximumHeight(150);
    serverLayout->addWidget(logArea);

    contentLayout->addWidget(serverWidget);

    // Footer
    QWidget* footerWidget = new QWidget();
    QVBoxLayout* footerLayout = new QVBoxLayout(footerWidget);
    footerLayout->setSpacing(4);

    footerLabel = new QLabel("NotY-Gemini-MCP v1.0.0 | Made with ❤️ by NotY215/Fliczo");
    footerLabel->setStyleSheet("color: " + DARK_TEXT_MUTED + "; font-size: 12px; text-align: center;");
    footerLabel->setAlignment(Qt::AlignCenter);
    footerLayout->addWidget(footerLabel);

    termsLink = new QPushButton("View Terms & Conditions");
    termsLink->setStyleSheet("QPushButton { background: transparent; color: " + DARK_ACCENT +
        "; border: none; font-size: 11px; text-decoration: underline; } "
        "QPushButton:hover { color: #7b8ff5; }");
    termsLink->setCursor(Qt::PointingHandCursor);
    connect(termsLink, &QPushButton::clicked, this, &AppWindow::onTermsClicked);
    footerLayout->addWidget(termsLink, 0, Qt::AlignCenter);

    mainLayout->addWidget(footerWidget);
    mainLayout->addStretch();

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);

    // Add initial logs
    addLog("🚀 Application initialized", "success");
    addLog("📋 Ready to start server", "info");
}

void AppWindow::setupTrayIcon() {
    trayIcon = std::make_unique<QSystemTrayIcon>(this);
    trayIcon->setIcon(QIcon(":/resources/tray-icon.png"));
    trayIcon->setToolTip("NotY-Gemini-MCP");

    trayMenu = std::make_unique<QMenu>(this);
    QAction* openAction = new QAction("Open NotY-Gemini-MCP", this);
    QAction* exitAction = new QAction("Exit", this);

    connect(openAction, &QAction::triggered, this, &AppWindow::show);
    connect(exitAction, &QAction::triggered, this, &AppWindow::onExitAction);

    trayMenu->addAction(openAction);
    trayMenu->addSeparator();
    trayMenu->addAction(exitAction);

    trayIcon->setContextMenu(trayMenu.get());
    connect(trayIcon.get(), &QSystemTrayIcon::activated, this, &AppWindow::onTrayActivated);
}

void AppWindow::addLog(const QString& message, const QString& type) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString formatted = QString("[%1] ").arg(timestamp);

    if (type == "success") {
        formatted += "✅ " + message;
    }
    else if (type == "error") {
        formatted += "❌ " + message;
    }
    else if (type == "warning") {
        formatted += "⚠️ " + message;
    }
    else {
        formatted += "ℹ️ " + message;
    }

    logArea->append(formatted);

    // Color the log based on type
    QTextCursor cursor = logArea->textCursor();
    cursor.movePosition(QTextCursor::End);
    QTextCharFormat format = cursor.charFormat();

    if (type == "success") {
        format.setForeground(QColor(DARK_SUCCESS));
    }
    else if (type == "error") {
        format.setForeground(QColor(DARK_DANGER));
    }
    else if (type == "warning") {
        format.setForeground(QColor(DARK_WARNING));
    }
    else {
        format.setForeground(QColor(DARK_TEXT_MUTED));
    }

    cursor.setCharFormat(format);
    logArea->setTextCursor(cursor);
    logArea->ensureCursorVisible();
}

void AppWindow::showToast(const QString& message, const QString& type) {
    QMessageBox* toast = new QMessageBox(this);
    toast->setText(message);
    toast->setStyleSheet("QMessageBox { background-color: " + DARK_BG_CARD + "; color: " + DARK_TEXT_PRIMARY + "; } "
        "QPushButton { background-color: " + DARK_ACCENT + "; color: white; border: none; "
        "padding: 6px 16px; border-radius: 6px; }");

    if (type == "success") {
        toast->setIcon(QMessageBox::Information);
    }
    else if (type == "error") {
        toast->setIcon(QMessageBox::Critical);
    }
    else if (type == "warning") {
        toast->setIcon(QMessageBox::Warning);
    }
    else {
        toast->setIcon(QMessageBox::Information);
    }

    toast->show();
    QTimer::singleShot(3000, toast, &QMessageBox::accept);
}

void AppWindow::updateVSCodeStatus(bool installed, const QString& path) {
    vscodeInstalled = installed;
    vscodePath = path;

    if (installed) {
        vscodeStatus->setText("✅ VS Code is installed");
        vscodeStatus->setStyleSheet("color: " + DARK_SUCCESS + "; padding: 8px 12px; "
            "background-color: rgba(72, 187, 120, 0.1); border-radius: 8px;");
        vscodePathLabel->setText("Path: " + path);
        vscodePathLabel->setVisible(true);
        apiKeyWidget->setVisible(true);
    }
    else {
        vscodeStatus->setText("❌ VS Code is not installed");
        vscodeStatus->setStyleSheet("color: " + DARK_DANGER + "; padding: 8px 12px; "
            "background-color: rgba(245, 101, 101, 0.1); border-radius: 8px;");
        vscodePathLabel->setVisible(false);
        apiKeyWidget->setVisible(false);
        serverWidget->setVisible(false);
    }
}

void AppWindow::updateApiKeyStatus(bool valid) {
    apiKeyValid = valid;

    if (valid) {
        apiKeyStatusLabel->setText("✅ API key verified successfully!");
        apiKeyStatusLabel->setStyleSheet("color: " + DARK_SUCCESS + "; font-size: 13px;");
        apiKeyStatusLabel->setVisible(true);
        saveApiKeyBtn->setEnabled(false);
        saveApiKeyBtn->setText("✓ Verified");
        apiKeyInput->setEnabled(false);
        serverWidget->setVisible(true);
        showToast("API key verified successfully!", "success");
        addLog("✅ API key verified successfully", "success");
    }
    else {
        apiKeyStatusLabel->setText("❌ Invalid API key. Please try again.");
        apiKeyStatusLabel->setStyleSheet("color: " + DARK_DANGER + "; font-size: 13px;");
        apiKeyStatusLabel->setVisible(true);
        saveApiKeyBtn->setEnabled(true);
        saveApiKeyBtn->setText("💾 Save & Verify");
        apiKeyInput->setEnabled(true);
        showToast("Invalid API key. Please check and try again.", "error");
        addLog("❌ Invalid API key", "error");
    }
}

void AppWindow::updateServerStatus(bool running) {
    serverRunning = running;

    if (running) {
        statusBadge->setText("● Server Running");
        statusBadge->setStyleSheet("font-size: 12px; font-weight: 500; padding: 4px 16px; "
            "border-radius: 12px; background-color: rgba(72, 187, 120, 0.2); "
            "color: " + DARK_SUCCESS + ";");
        serverStatusLabel->setText("Status: 🟢 Running");
        serverStatusLabel->setStyleSheet("color: " + DARK_SUCCESS + "; font-weight: 500;");
        startServerBtn->setEnabled(false);
        startServerBtn->setText("▶ Running");
        stopServerBtn->setEnabled(true);
        if (trayIcon) {
            trayIcon->show();
        }
        showToast("Server started successfully!", "success");
        addLog("🚀 Server started successfully", "success");
    }
    else {
        statusBadge->setText("● Server Stopped");
        statusBadge->setStyleSheet("font-size: 12px; font-weight: 500; padding: 4px 16px; "
            "border-radius: 12px; background-color: rgba(245, 101, 101, 0.2); "
            "color: " + DARK_DANGER + ";");
        serverStatusLabel->setText("Status: 🔴 Stopped");
        serverStatusLabel->setStyleSheet("color: " + DARK_DANGER + "; font-weight: 500;");
        startServerBtn->setEnabled(true);
        startServerBtn->setText("▶ Start Server");
        stopServerBtn->setEnabled(false);
        if (trayIcon) {
            trayIcon->hide();
        }
        showToast("Server stopped.", "info");
        addLog("⏹️ Server stopped", "info");
    }
}

bool AppWindow::create() {
    // Set icon
    setWindowIcon(QIcon(":/resources/icon.png"));
    return true;
}

void AppWindow::show() {
    QMainWindow::show();
    raise();
    activateWindow();
}

void AppWindow::hide() {
    QMainWindow::hide();
}

void AppWindow::close() {
    QMainWindow::close();
}

void AppWindow::setTitle(const std::string& title) {
    setWindowTitle(QString::fromStdString(title));
}

void AppWindow::setIcon(const std::string& path) {
    setWindowIcon(QIcon(QString::fromStdString(path)));
}

void AppWindow::setOnClose(std::function<void()> callback) {
    onCloseCallback = callback;
}

void AppWindow::setOnWebMessage(std::function<void(const std::string&)> callback) {
    onWebMessageCallback = callback;
}

void AppWindow::sendToWeb(const std::string& message) {
    // This is now handled via Qt signals
    emit qmlMessage(QString::fromStdString(message));
}

void AppWindow::closeEvent(QCloseEvent* event) {
    if (serverRunning) {
        event->ignore();
        hide();
        trayIcon->show();
        showToast("Application minimized to system tray", "info");
        addLog("📌 Application minimized to tray", "info");
    }
    else {
        if (onCloseCallback) {
            onCloseCallback();
        }
        event->accept();
    }
}

void AppWindow::onBrowseVSCode() {
    QString filePath = QFileDialog::getOpenFileName(this,
        "Select VS Code Executable",
        "C:\\Program Files",
        "Executable Files (*.exe);;All Files (*.*)");

    if (!filePath.isEmpty()) {
        if (onWebMessageCallback) {
            nlohmann::json msg;
            msg["type"] = "browse-vscode";
            msg["path"] = filePath.toStdString();
            onWebMessageCallback(msg.dump());
        }
    }
}

void AppWindow::onRefreshVSCode() {
    if (onWebMessageCallback) {
        nlohmann::json msg;
        msg["type"] = "refresh-vscode";
        onWebMessageCallback(msg.dump());
    }
    addLog("🔄 Refreshing VS Code detection...", "info");
}

void AppWindow::onDownloadVSCode() {
    QDesktopServices::openUrl(QUrl("https://code.visualstudio.com/download?_exp_download=fb315fc982"));
    addLog("📥 Opening VS Code download page...", "info");
}

void AppWindow::onSaveApiKey() {
    QString key = apiKeyInput->text().trimmed();
    if (key.isEmpty()) {
        showToast("Please enter a valid API key", "warning");
        return;
    }

    saveApiKeyBtn->setEnabled(false);
    saveApiKeyBtn->setText("⏳ Verifying...");
    addLog("⏳ Verifying API key...", "info");

    if (onWebMessageCallback) {
        nlohmann::json msg;
        msg["type"] = "save-api-key";
        msg["key"] = key.toStdString();
        onWebMessageCallback(msg.dump());
    }
}

void AppWindow::onGetApiKey() {
    QDesktopServices::openUrl(QUrl("https://aistudio.google.com/api-keys"));
    addLog("🔑 Opening Google AI Studio for API key...", "info");
}

void AppWindow::onStartServer() {
    if (onWebMessageCallback) {
        nlohmann::json msg;
        msg["type"] = "start-server";
        onWebMessageCallback(msg.dump());
    }
    addLog("⏳ Starting server...", "info");
}

void AppWindow::onStopServer() {
    if (onWebMessageCallback) {
        nlohmann::json msg;
        msg["type"] = "stop-server";
        onWebMessageCallback(msg.dump());
    }
    addLog("⏳ Stopping server...", "info");
}

void AppWindow::onTermsClicked() {
    QMessageBox::information(this, "Terms & Conditions",
        "⚠️ DISCLAIMER OF LIABILITY\n\n"
        "The developers and contributors of NotY-Gemini-MCP are NOT responsible for any data breaches, "
        "security incidents, or unauthorized access to your systems. This software is provided 'AS IS' "
        "and you use it entirely at your own risk.\n\n"
        "By using this software, you agree to:\n"
        "• Keep your Gemini API key secure\n"
        "• Review all AI-generated code before implementing\n"
        "• Not use the software for illegal purposes\n"
        "• Accept all risks associated with AI-assisted coding\n\n"
        "Full terms available at: https://github.com/NotY215/NotY-Gemini-mcp");
}

void AppWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        show();
    }
}

void AppWindow::onExitAction() {
    if (onCloseCallback) {
        onCloseCallback();
    }
    close();
}

void AppWindow::onShowKeyToggled(bool checked) {
    apiKeyInput->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}

// Emit signal for QML communication
void AppWindow::qmlMessage(const QString& message) {
    // This is a placeholder for Qt signal/slot mechanism
}