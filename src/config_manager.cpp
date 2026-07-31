#include "config_manager.h"
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

ConfigManager::ConfigManager() : loaded(false) {
    configPath = getConfigPath();
}

ConfigManager::~ConfigManager() {
    if (loaded) {
        save();
    }
}

std::string ConfigManager::getConfigPath() const {
    char appDataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
        std::string path = std::string(appDataPath) + "\\gemini-mcp";
        if (!fs::exists(path)) {
            fs::create_directories(path);
        }
        return path + "\\config.ini";
    }
    return "config.ini";
}

bool ConfigManager::load() {
    try {
        if (fs::exists(configPath)) {
            boost::property_tree::ini_parser::read_ini(configPath, config);
            loaded = true;
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
    }
    
    // Create default config if not exists
    createDefaultConfig();
    return load();
}

bool ConfigManager::save() {
    try {
        boost::property_tree::ini_parser::write_ini(configPath, config);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving config: " << e.what() << std::endl;
        return false;
    }
}

void ConfigManager::createDefaultConfig() {
    config.put("api.key", "");
    config.put("vscode.path", "");
    config.put("server.running", false);
    config.put("terms.accepted", false);
    config.put("server.port", 31415);
    loaded = true;
    save();
}

void ConfigManager::setDefaults() {
    config.put("api.key", "");
    config.put("vscode.path", "");
    config.put("server.running", false);
    config.put("terms.accepted", false);
    config.put("server.port", 31415);
    loaded = true;
    save();
}

std::string ConfigManager::getApiKey() const {
    return config.get<std::string>("api.key", "");
}

void ConfigManager::setApiKey(const std::string& key) {
    config.put("api.key", key);
    save();
}

std::string ConfigManager::getVSCodePath() const {
    return config.get<std::string>("vscode.path", "");
}

void ConfigManager::setVSCodePath(const std::string& path) {
    config.put("vscode.path", path);
    save();
}

bool ConfigManager::getServerRunning() const {
    return config.get<bool>("server.running", false);
}

void ConfigManager::setServerRunning(bool running) {
    config.put("server.running", running);
    save();
}

bool ConfigManager::getTermsAccepted() const {
    return config.get<bool>("terms.accepted", false);
}

void ConfigManager::setTermsAccepted(bool accepted) {
    config.put("terms.accepted", accepted);
    save();
}

int ConfigManager::getPort() const {
    return config.get<int>("server.port", 31415);
}

void ConfigManager::setPort(int port) {
    config.put("server.port", port);
    save();
}