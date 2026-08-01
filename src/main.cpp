#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <shlobj.h>
#include <commdlg.h>
#include <nlohmann/json.hpp>
#include "app_window.h"
#include "web_server.h"
#include "tray_icon.h"
#include "config_manager.h"
#include "logger.h"
#include "vscode_manager.h"
#include "gemini_service.h"
#include "encryption.h"

class Application {
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

            runMessageLoop();

            return 0;
        }
        catch (const std::exception& e) {
            logger->error("Exception: {}", e.what());
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
        window->setIcon("resources/icon.ico");

        window->setOnClose([this]() {
            if (config->getServerRunning()) {
                window->hide();
            }
            else {
                isRunning = false;
                PostQuitMessage(0);
            }
            });

        return window->create();
    }

    bool createTrayIcon() {
        logger->info("Creating tray icon...");
        trayIcon = std::make_unique<TrayIcon>();

        trayIcon->setIcon("resources/tray-icon.png");
        trayIcon->setTooltip("NotY-Gemini-MCP - Server Running");

        trayIcon->onOpen([this]() {
            if (window) {
                window->show();
            }
            });

        trayIcon->onExit([this]() {
            stopServer();
            isRunning = false;
            PostQuitMessage(0);
            });

        return trayIcon->create();
    }

    void checkVSCode() {
        logger->info("Checking VS Code installation...");

        auto vscodePath = vscodeManager->findInstallation();
        if (vscodePath.has_value()) {
            config->setVSCodePath(vscodePath.value());
            logger->info("VS Code found at: {}", vscodePath.value());

            nlohmann::json response;
            response["type"] = "vscode-status";
            response["installed"] = true;
            response["path"] = vscodePath.value();
            sendToWebUI(response.dump());
        }
        else {
            logger->warn("VS Code not found");
            nlohmann::json response;
            response["type"] = "vscode-status";
            response["installed"] = false;
            sendToWebUI(response.dump());
        }
    }

    void handleSaveApiKey(const std::string& apiKey) {
        logger->info("Saving API key...");

        try {
            std::string encrypted = Encryption::encrypt(apiKey);
            config->setApiKey(encrypted);

            geminiService = std::make_unique<GeminiService>(apiKey);

            if (geminiService->verifyKey()) {
                nlohmann::json response;
                response["type"] = "api-key-valid";
                response["valid"] = true;
                sendToWebUI(response.dump());

                logger->info("API key verified successfully");
            }
            else {
                config->setApiKey("");
                nlohmann::json response;
                response["type"] = "api-key-valid";
                response["valid"] = false;
                sendToWebUI(response.dump());

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

        nlohmann::json response;
        response["type"] = "server-started";
        sendToWebUI(response.dump());

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

        nlohmann::json response;
        response["type"] = "server-stopped";
        sendToWebUI(response.dump());

        logger->info("Server stopped");
    }

    void handleBrowseVSCode() {
        OPENFILENAMEA ofn = { 0 };
        char fileName[MAX_PATH] = { 0 };

        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = window->getHandle();
        ofn.lpstrFilter = "Executable Files\0*.exe\0All Files\0*.*\0";
        ofn.lpstrFile = fileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

        if (GetOpenFileNameA(&ofn)) {
            std::string selectedPath(fileName);
            if (vscodeManager->validateExecutable(selectedPath)) {
                config->setVSCodePath(selectedPath);
                checkVSCode();
            }
        }
    }

    void sendToWebUI(const std::string& message) {
        if (window) {
            window->sendToWeb(message);
        }
    }

    void runMessageLoop() {
        MSG msg;
        while (isRunning && GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);

    try {
        Application app;
        return app.run();
    }
    catch (const std::exception& e) {
        MessageBoxA(NULL, e.what(), "Fatal Error", MB_OK | MB_ICONERROR);
        return 1;
    }
}