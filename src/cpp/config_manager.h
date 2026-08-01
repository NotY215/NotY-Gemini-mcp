#pragma once
#include <string>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

class ConfigManager {
private:
    std::string configPath;
    boost::property_tree::ptree config;
    bool loaded;

public:
    ConfigManager();
    ~ConfigManager();

    bool load();
    bool save();
    void setDefaults();

    // Getters and Setters
    std::string getApiKey() const;
    void setApiKey(const std::string& key);
    
    std::string getVSCodePath() const;
    void setVSCodePath(const std::string& path);
    
    bool getServerRunning() const;
    void setServerRunning(bool running);
    
    bool getTermsAccepted() const;
    void setTermsAccepted(bool accepted);
    
    int getPort() const;
    void setPort(int port);

private:
    std::string getConfigPath() const;
    void createDefaultConfig();
};