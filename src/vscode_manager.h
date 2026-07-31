#pragma once
#include <string>
#include <vector>
#include <optional>
#include <windows.h>

class VSCodeManager {
private:
    std::string detectedPath;
    std::vector<std::string> getPossiblePaths() const;
    bool isVSCodeExecutable(const std::string& path) const;

public:
    VSCodeManager();
    ~VSCodeManager();

    std::optional<std::string> findInstallation();
    bool validateExecutable(const std::string& path) const;
    std::string getVSCodePath() const { return detectedPath; }
    bool isInstalled() const { return !detectedPath.empty(); }

    // Run VS Code with specific arguments
    bool launchVSCode(const std::string& projectPath = "");
    bool openTerminal(const std::string& projectPath = "");
    
    // Get VS Code version
    std::string getVersion() const;
};