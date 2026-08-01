#pragma once
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

class GeminiService {
private:
    std::string apiKey;
    bool valid;
    std::string baseUrl;
    std::string model;
    std::string chatHistory;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    std::string makeRequest(const std::string& endpoint, const nlohmann::json& payload);
    nlohmann::json createPrompt(const std::string& message, const std::string& context = "");

public:
    GeminiService(const std::string& key);
    ~GeminiService();

    bool verifyKey();
    bool isValid() const { return valid; }
    
    std::string sendMessage(const std::string& message, const std::string& context = "");
    std::string analyzeCode(const std::string& code, const std::string& question);
    std::string fixErrors(const std::string& errorLog, const std::string& code);
    void clearHistory();

private:
    void initCurl();
    static bool curlInitialized;
};