#include "vscode_manager.h"
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <windows.h>

namespace fs = std::filesystem;

VSCodeManager::VSCodeManager() {}

VSCodeManager::~VSCodeManager() {}

std::vector<std::string> VSCodeManager::getPossiblePaths() const {
    std::vector<std::string> paths;
    
    // Windows paths
    char programFiles[MAX_PATH];
    char localAppData[MAX_PATH];
    
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILES, NULL, 0, programFiles))) {
        paths.push_back(std::string(programFiles) + "\\Microsoft VS Code\\Code.exe");
        paths.push_back(std::string(programFiles) + "\\Microsoft VS Code Insiders\\Code - Insiders.exe");
    }
    
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, programFiles))) {
        paths.push_back(std::string(programFiles) + "\\Microsoft VS Code\\Code.exe");
    }
    
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData))) {
        paths.push_back(std::string(localAppData) + "\\Programs\\Microsoft VS Code\\Code.exe");
        paths.push_back(std::string(localAppData) + "\\Programs\\Microsoft VS Code Insiders\\Code - Insiders.exe");
    }
    
    // Check PATH environment variable
    const char* pathEnv = getenv("PATH");
    if (pathEnv) {
        std::string pathStr(pathEnv);
        std::string delimiter = ";";
        size_t pos = 0;
        while ((pos = pathStr.find(delimiter)) != std::string::npos) {
            std::string dir = pathStr.substr(0, pos);
            paths.push_back(dir + "\\Code.exe");
            paths.push_back(dir + "\\code.exe");
            pathStr.erase(0, pos + delimiter.length());
        }
        paths.push_back(pathStr + "\\Code.exe");
        paths.push_back(pathStr + "\\code.exe");
    }
    
    return paths;
}

bool VSCodeManager::isVSCodeExecutable(const std::string& path) const {
    if (!fs::exists(path)) {
        return false;
    }
    
    std::string filename = fs::path(path).filename().string();
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
    
    if (filename.find("code") == std::string::npos) {
        return false;
    }
    
    // Try to get version info to verify it's VS Code
    DWORD handle;
    DWORD size = GetFileVersionInfoSizeA(path.c_str(), &handle);
    if (size > 0) {
        std::vector<char> buffer(size);
        if (GetFileVersionInfoA(path.c_str(), handle, size, buffer.data())) {
            VS_FIXEDFILEINFO* fileInfo;
            UINT len;
            if (VerQueryValueA(buffer.data(), "\\", (LPVOID*)&fileInfo, &len)) {
                // Check if it's Microsoft's VS Code
                if (fileInfo->dwFileVersionMS != 0) {
                    return true;
                }
            }
        }
    }
    
    // Fallback: check if file is executable
    return true;
}

std::optional<std::string> VSCodeManager::findInstallation() {
    auto paths = getPossiblePaths();
    
    for (const auto& path : paths) {
        if (isVSCodeExecutable(path)) {
            detectedPath = path;
            return detectedPath;
        }
    }
    
    // Try using 'where' command as fallback
    FILE* pipe = _popen("where code 2>nul", "r");
    if (pipe) {
        char buffer[256];
        std::string result;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        _pclose(pipe);
        
        if (!result.empty()) {
            // Take first line
            size_t pos = result.find('\n');
            if (pos != std::string::npos) {
                result = result.substr(0, pos);
            }
            // Remove trailing \r
            if (!result.empty() && result.back() == '\r') {
                result.pop_back();
            }
            
            if (isVSCodeExecutable(result)) {
                detectedPath = result;
                return detectedPath;
            }
        }
    }
    
    return std::nullopt;
}

bool VSCodeManager::validateExecutable(const std::string& path) const {
    return isVSCodeExecutable(path);
}

bool VSCodeManager::launchVSCode(const std::string& projectPath) {
    if (detectedPath.empty()) {
        return false;
    }
    
    std::string command = "\"" + detectedPath + "\"";
    if (!projectPath.empty()) {
        command += " \"" + projectPath + "\"";
    }
    
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    
    if (CreateProcessA(NULL, (LPSTR)command.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    
    return false;
}

bool VSCodeManager::openTerminal(const std::string& projectPath) {
    if (detectedPath.empty()) {
        return false;
    }
    
    std::string command = "\"" + detectedPath + "\" --new-window --terminal";
    if (!projectPath.empty()) {
        command += " \"" + projectPath + "\"";
    }
    
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    
    if (CreateProcessA(NULL, (LPSTR)command.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    
    return false;
}

std::string VSCodeManager::getVersion() const {
    if (detectedPath.empty()) {
        return "";
    }
    
    DWORD handle;
    DWORD size = GetFileVersionInfoSizeA(detectedPath.c_str(), &handle);
    if (size > 0) {
        std::vector<char> buffer(size);
        if (GetFileVersionInfoA(detectedPath.c_str(), handle, size, buffer.data())) {
            VS_FIXEDFILEINFO* fileInfo;
            UINT len;
            if (VerQueryValueA(buffer.data(), "\\", (LPVOID*)&fileInfo, &len)) {
                char version[64];
                snprintf(version, sizeof(version), "%d.%d.%d.%d",
                    HIWORD(fileInfo->dwFileVersionMS),
                    LOWORD(fileInfo->dwFileVersionMS),
                    HIWORD(fileInfo->dwFileVersionLS),
                    LOWORD(fileInfo->dwFileVersionLS));
                return std::string(version);
            }
        }
    }
    
    return "";
}