#include <QApplication>
#include <QMessageBox>
#include <QIcon>
#include <memory>
#include <iostream>
#include <thread>
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

            if (!createTrayIcon()) {
                logger->error("Failed to create tray icon");
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
        window = std::make_unique<AppWindow>(1000, 700);
        
        window->setTitle("NotY-Gemini-MCP");
        window->setIcon(":/resources/icon.png");
        
        // Connect QML signals
        QObject::connect(window.get(), &AppWindow::qmlMessage,
                         this, &Application::handleQmlMessage);
        
        window->setOnClose([this]() {
            if (config->getServerRunning()) {
                window->hide();
            } else {
                isRunning = false;
                QApplication::quit();
            }
        });

        window->show();
        return true;
    }

    bool createTrayIcon() {
        logger->info("Creating tray icon...");
        trayIcon = std::make_unique<TrayIcon>();
        
        trayIcon->setIcon(":/resources/tray-icon.png");
        trayIcon->setTooltip("NotY-Gemini-MCP - Server Running");
        
        trayIcon->onOpen([this]() {
            if (window) {
                window->show();
            }
        });

        trayIcon->onExit([this]() {
            stopServer();
            isRunning = false;
            QApplication::quit();
        });

        return trayIcon->create();
    }

    void checkVSCode() {
        logger->info("Checking VS Code installation...");
        
        auto vscodePath = vscodeManager->findInstallation();
        if (vscodePath.has_value()) {
            config->setVSCodePath(vscodePath.value());
            logger->info("VS Code found at: {}", vscodePath.value());
            sendToQml("vscode-status", {
                {"installed", true},
                {"path", vscodePath.value()}
            });
        } else {
            logger->warn("VS Code not found");
            sendToQml("vscode-status", {
                {"installed", false}
            });
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
                handleBrowseVSCode();
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
                sendToQml("api-key-valid", {
                    {"valid", true}
                });
                logger->info("API key verified successfully");
            } else {
                config->setApiKey("");
                sendToQml("api-key-valid", {
                    {"valid", false}
                });
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
        
        if (trayIcon) {
            trayIcon->show();
        }
        
        sendToQml("server-started", {});
        logger->info("Server started successfully");
    }

    void handleStopServer() {
        stopServer();
    }

    void stopServer() {
        logger->info("Stopping server...");
        config->setServerRunning(false);
        
        if (trayIcon) {
            trayIcon->hide();
        }
        
        sendToQml("server-stopped", {});
        logger->info("Server stopped");
    }

    void handleBrowseVSCode() {
        // Implement file dialog to browse for VS Code
        // This will be handled in QML
    }

    void sendToQml(const std::string& type, const nlohmann::json& data) {
        if (window) {
            nlohmann::json message;
            message["type"] = type;
            for (auto& [key, value] : data.items()) {
                message[key] = value;
            }
            window->sendToWeb(message.dump());
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application icon
    app.setWindowIcon(QIcon(":/resources/icon.png"));
    
    Application application;
    return application.run();
}

#include "main.moc"