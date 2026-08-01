#include <QApplication>
#include <QMessageBox>
#include <QIcon>
#include <memory>
#include <iostream>
#include "app_window.h"
#include "web_server.h"
#include "tray_icon.h"
#include "config_manager.h"
#include "logger.h"
#include "vscode_manager.h"
#include "gemini_service.h"
#include "encryption.h"

class Application : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<AppWindow> window;
    std::unique_ptr<WebServer> webServer;
    std::unique_ptr<TrayIcon> trayIcon;
    std::unique_ptr<ConfigManager> config;
    std::unique_ptr<VSCodeManager> vscodeManager;
    std::unique_ptr<GeminiService> geminiService;
    std::unique_ptr<Logger> logger;
    bool isRunning;

public:
    Application() : isRunning(true) {
        logger = std::make_unique<Logger>("app.log");
        config = std::make_unique<ConfigManager>();
        vscodeManager = std::make_unique<VSCodeManager>();
        Encryption::init();

        logger->info("Application starting...");

        // Set application info
        QApplication::setApplicationName("NotY-Gemini-MCP");
        QApplication::setOrganizationName("NotY");
        QApplication::setApplicationDisplayName("NotY-Gemini-MCP");
    }

    ~Application() {
        logger->info("Application shutting down...");
    }

    int run() {
        try {
            if (!config->load()) {
                config->setDefaults();
            }

            if (!startWebServer()) {
                logger->error("Failed to start web server");
                return 1;
            }

            if (!createMainWindow()) {
                logger->error("Failed to create main window");
                return 1;
            }

            checkVSCode();

            return QApplication::exec();
        }
        catch (const std::exception& e) {
            logger->error("Exception: {}", e.what());
            QMessageBox::critical(nullptr, "Fatal Error", e.what());
            return 1;
        }
    }

private:
    bool startWebServer() {
        logger->info("Starting web server...");
        webServer = std::make_unique<WebServer>(31415);

        webServer->setChatHandler([this](const std::string& message, const std::string& context) {
            if (geminiService && geminiService->isValid()) {
                return geminiService->sendMessage(message, context);
            }
            return std::string("Error: Gemini service not initialized");
            });

        webServer->setAnalyzeHandler([this](const std::string& code, const std::string& question) {
            if (geminiService && geminiService->isValid()) {
                return geminiService->analyzeCode(code, question);
            }
            return std::string("Error: Gemini service not initialized");
            });

        webServer->setFixErrorsHandler([this](const std::string& errorLog, const std::string& code) {
            if (geminiService && geminiService->isValid()) {
                return geminiService->fixErrors(errorLog, code);
            }
            return std::string("Error: Gemini service not initialized");
            });

        return webServer->start();
    }

    bool createMainWindow() {
        logger->info("Creating main window...");
        window = std::make_unique<AppWindow>();
        window->setWindowTitle("NotY-Gemini-MCP");
        window->setWindowIcon(QIcon(":/resources/icon.png"));
        window->resize(920, 700);

        // Connect signals
        QObject::connect(window.get(), &AppWindow::qmlMessage,
            this, &Application::handleQmlMessage);

        window->setOnClose([this]() {
            if (config->getServerRunning()) {
                window->hide();
            }
            else {
                isRunning = false;
                QApplication::quit();
            }
            });

        window->setOnWebMessage([this](const std::string& message) {
            handleQmlMessage(QString::fromStdString(message));
            });

        window->show();
        return true;
    }

    void checkVSCode() {
        logger->info("Checking VS Code installation...");

        auto vscodePath = vscodeManager->findInstallation();
        if (vscodePath.has_value()) {
            config->setVSCodePath(vscodePath.value());
            logger->info("VS Code found at: {}", vscodePath.value());
            window->updateVSCodeStatus(true, QString::fromStdString(vscodePath.value()));
        }
        else {
            logger->warn("VS Code not found");
            window->updateVSCodeStatus(false, "");
        }
    }

    void handleQmlMessage(const QString& qmlMessage) {
        try {
            std::string message = qmlMessage.toStdString();
            auto json = nlohmann::json::parse(message);
            std::string type = json["type"];

            if (type == "save-api-key") {
                handleSaveApiKey(json["key"]);
            }
            else if (type == "start-server") {
                handleStartServer();
            }
            else if (type == "stop-server") {
                handleStopServer();
            }
            else if (type == "browse-vscode") {
                handleBrowseVSCode(json["path"]);
            }
            else if (type == "refresh-vscode") {
                checkVSCode();
            }
            else if (type == "accept-terms") {
                config->setTermsAccepted(true);
            }
            else if (type == "decline-terms") {
                QApplication::quit();
            }
        }
        catch (const std::exception& e) {
            logger->error("Error handling QML message: {}", e.what());
        }
    }

    void handleSaveApiKey(const std::string& apiKey) {
        logger->info("Saving API key...");

        try {
            std::string encrypted = Encryption::encrypt(apiKey);
            config->setApiKey(encrypted);

            geminiService = std::make_unique<GeminiService>(apiKey);

            if (geminiService->verifyKey()) {
                window->updateApiKeyStatus(true);
                logger->info("API key verified successfully");
            }
            else {
                config->setApiKey("");
                window->updateApiKeyStatus(false);
                logger->error("Invalid API key");
            }
        }
        catch (const std::exception& e) {
            logger->error("Error saving API key: {}", e.what());
        }
    }

    void handleStartServer() {
        logger->info("Starting server...");
        config->setServerRunning(true);
        window->updateServerStatus(true);
        logger->info("Server started successfully");
    }

    void handleStopServer() {
        logger->info("Stopping server...");
        config->setServerRunning(false);
        window->updateServerStatus(false);
        logger->info("Server stopped");
    }

    void handleBrowseVSCode(const std::string& path) {
        if (!path.empty() && vscodeManager->validateExecutable(path)) {
            config->setVSCodePath(path);
            window->updateVSCodeStatus(true, QString::fromStdString(path));
            logger->info("VS Code path set to: {}", path);
        }
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // Set application icon
    app.setWindowIcon(QIcon(":/resources/icon.png"));

    Application application;
    return application.run();
}

#include "main.moc"