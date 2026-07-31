#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include "app_window.h"
#include "web_server.h"
#include "tray_icon.h"
#include "config_manager.h"
#include "logger.h"
#include "vscode_manager.h"
#include "gemini_service.h"

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
        
        logger->info("Application starting...");
    }

    ~Application() {
        logger->info("Application shutting down...");
    }

    int run() {
        try {
            // Initialize components
            if (!initializeComponents()) {
                logger->error("Failed to initialize components");
                return 1;
            }

            // Start web server
            if (!startWebServer()) {
                logger->error("Failed to start web server");
                return 1;
            }

            // Create main window
            if (!createMainWindow()) {
                logger->error("Failed to create main window");
                return 1;
            }

            // Create tray icon
            if (!createTrayIcon()) {
                logger->error("Failed to create tray icon");
                return 1;
            }

            // Check VS Code
            checkVSCode();

            // Message loop
            runMessageLoop();

            return 0;
        }
        catch (const std::exception& e) {
            logger->error("Exception: {}", e.what());
            return 1;
        }
    }

private:
    bool initializeComponents() {
        logger->info("Initializing components...");
        
        // Load configuration
        if (!config->load()) {
            logger->warn("No configuration found, using defaults");
            config->setDefaults();
        }

        // Initialize encryption
        Encryption::init();

        return true;
    }

    bool startWebServer() {
        logger->info("Starting web server...");
        webServer = std::make_unique<WebServer>(31415);
        
        // Set up web server handlers
        webServer->onChat([this](const std::string& message, const std::string& context) {
            if (geminiService && geminiService->isValid()) {
                return geminiService->sendMessage(message, context);
            }
            return std::string("Error: Gemini service not initialized");
        });

        webServer->onAnalyze([this](const std::string& code, const std::string& question) {
            if (geminiService && geminiService->isValid()) {
                return geminiService->analyzeCode(code, question);
            }
            return std::string("Error: Gemini service not initialized");
        });

        webServer->onFixErrors([this](const std::string& errorLog, const std::string& code) {
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
        window->setWebHandler([this](const std::string& path) {
            return serveWebFile(path);
        });

        // Set up window callbacks
        window->onClose([this]() {
            if (config->getServerRunning()) {
                // Minimize to tray instead of closing
                window->hide();
            } else {
                isRunning = false;
                PostQuitMessage(0);
            }
        });

        window->onWebMessage([this](const std::string& message) {
            handleWebMessage(message);
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
            
            // Send status to UI
            nlohmann::json response;
            response["type"] = "vscode-status";
            response["installed"] = true;
            response["path"] = vscodePath.value();
            sendToWebUI(response.dump());
        } else {
            logger->warn("VS Code not found");
            nlohmann::json response;
            response["type"] = "vscode-status";
            response["installed"] = false;
            sendToWebUI(response.dump());
        }
    }

    void handleWebMessage(const std::string& message) {
        try {
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
            else if (type == "get-status") {
                sendStatus();
            }
        }
        catch (const std::exception& e) {
            logger->error("Error handling web message: {}", e.what());
        }
    }

    void handleSaveApiKey(const std::string& apiKey) {
        logger->info("Saving API key...");
        
        try {
            std::string encrypted = Encryption::encrypt(apiKey);
            config->setApiKey(encrypted);
            
            // Initialize Gemini service
            geminiService = std::make_unique<GeminiService>(apiKey);
            
            if (geminiService->verifyKey()) {
                nlohmann::json response;
                response["type"] = "api-key-valid";
                response["valid"] = true;
                sendToWebUI(response.dump());
                
                logger->info("API key verified successfully");
            } else {
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
        
        if (!geminiService || !geminiService->isValid()) {
            logger->error("Cannot start server: Gemini service not initialized");
            return;
        }

        config->setServerRunning(true);
        
        nlohmann::json response;
        response["type"] = "server-started";
        sendToWebUI(response.dump());
        
        // Show tray icon
        if (trayIcon) {
            trayIcon->show();
        }
        
        logger->info("Server started successfully");
    }

    void handleStopServer() {
        logger->info("Stopping server...");
        
        config->setServerRunning(false);
        
        nlohmann::json response;
        response["type"] = "server-stopped";
        sendToWebUI(response.dump());
        
        // Hide tray icon
        if (trayIcon) {
            trayIcon->hide();
        }
        
        logger->info("Server stopped");
    }

    void handleBrowseVSCode() {
        // Open file dialog to select VS Code executable
        OPENFILENAME ofn = {0};
        char fileName[MAX_PATH] = {0};
        
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = window->getHandle();
        ofn.lpstrFilter = "Executable Files\0*.exe\0All Files\0*.*\0";
        ofn.lpstrFile = fileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
        
        if (GetOpenFileName(&ofn)) {
            std::string selectedPath(fileName);
            if (vscodeManager->validateExecutable(selectedPath)) {
                config->setVSCodePath(selectedPath);
                checkVSCode();
            }
        }
    }

    void sendStatus() {
        nlohmann::json response;
        response["type"] = "status";
        response["serverRunning"] = config->getServerRunning();
        response["hasApiKey"] = !config->getApiKey().empty();
        response["vscodePath"] = config->getVSCodePath();
        sendToWebUI(response.dump());
    }

    void sendToWebUI(const std::string& message) {
        if (window) {
            window->sendToWeb(message);
        }
    }

    std::string serveWebFile(const std::string& path) {
        std::string filePath = "web" + path;
        if (path == "/") filePath = "web/index.html";
        
        std::ifstream file(filePath, std::ios::binary);
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
            return content;
        }
        return "";
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
    // Set console for debugging
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