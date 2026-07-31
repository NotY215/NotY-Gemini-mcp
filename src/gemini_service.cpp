#include "gemini_service.h"
#include <sstream>
#include <iostream>
#include <regex>

bool GeminiService::curlInitialized = false;

GeminiService::GeminiService(const std::string& key) 
    : apiKey(key), valid(false), baseUrl("https://generativelanguage.googleapis.com/v1beta"), 
      model("gemini-pro") {
    initCurl();
}

GeminiService::~GeminiService() {
    // Cleanup if needed
}

void GeminiService::initCurl() {
    if (!curlInitialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curlInitialized = true;
    }
}

size_t GeminiService::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool GeminiService::verifyKey() {
    try {
        // Send a simple test message
        nlohmann::json testPayload = {
            {"contents", {
                {{"parts", {{{"text", "Hello"}}}}}
            }}
        };
        
        std::string response = makeRequest("models/" + model + ":generateContent", testPayload);
        
        if (!response.empty()) {
            auto jsonResponse = nlohmann::json::parse(response);
            if (jsonResponse.contains("candidates") && !jsonResponse["candidates"].empty()) {
                valid = true;
                return true;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "API key verification failed: " << e.what() << std::endl;
    }
    
    valid = false;
    return false;
}

std::string GeminiService::makeRequest(const std::string& endpoint, const nlohmann::json& payload) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    std::string response;
    std::string url = baseUrl + "/" + endpoint + "?key=" + apiKey;
    std::string jsonPayload = payload.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    return response;
}

nlohmann::json GeminiService::createPrompt(const std::string& message, const std::string& context) {
    std::string fullMessage = message;
    if (!context.empty()) {
        fullMessage = "Context: " + context + "\n\n" + message;
    }

    nlohmann::json payload = {
        {"contents", {
            {{"parts", {{{"text", fullMessage}}}}}
        }},
        {"generationConfig", {
            {"temperature", 0.7},
            {"maxOutputTokens", 2048},
            {"topP", 0.8},
            {"topK", 40}
        }}
    };

    // Add safety settings
    nlohmann::json safetySettings = {
        {{"category", "HARM_CATEGORY_HARASSMENT"}, {"threshold", "BLOCK_MEDIUM_AND_ABOVE"}},
        {{"category", "HARM_CATEGORY_HATE_SPEECH"}, {"threshold", "BLOCK_MEDIUM_AND_ABOVE"}},
        {{"category", "HARM_CATEGORY_SEXUALLY_EXPLICIT"}, {"threshold", "BLOCK_MEDIUM_AND_ABOVE"}},
        {{"category", "HARM_CATEGORY_DANGEROUS_CONTENT"}, {"threshold", "BLOCK_MEDIUM_AND_ABOVE"}}
    };
    payload["safetySettings"] = safetySettings;

    return payload;
}

std::string GeminiService::sendMessage(const std::string& message, const std::string& context) {
    if (!valid) {
        return "Error: API key not verified";
    }

    try {
        auto payload = createPrompt(message, context);
        std::string response = makeRequest("models/" + model + ":generateContent", payload);
        
        if (!response.empty()) {
            auto jsonResponse = nlohmann::json::parse(response);
            if (jsonResponse.contains("candidates") && !jsonResponse["candidates"].empty()) {
                auto& candidate = jsonResponse["candidates"][0];
                if (candidate.contains("content") && candidate["content"].contains("parts")) {
                    auto& parts = candidate["content"]["parts"];
                    if (!parts.empty() && parts[0].contains("text")) {
                        std::string text = parts[0]["text"];
                        // Store in history
                        chatHistory += "User: " + message + "\n";
                        chatHistory += "Assistant: " + text + "\n\n";
                        return text;
                    }
                }
            }
        }
        return "Error: Failed to get response from Gemini";
    } catch (const std::exception& e) {
        return "Error: " + std::string(e.what());
    }
}

std::string GeminiService::analyzeCode(const std::string& code, const std::string& question) {
    std::string prompt = 
        "You are a coding assistant integrated with VS Code. You have access to the current project files.\n\n"
        "Current Code:\n```\n" + code + "\n```\n\n"
        "Question: " + question + "\n\n"
        "Please provide a comprehensive analysis, including:\n"
        "1. Code quality assessment\n"
        "2. Potential bugs or issues\n"
        "3. Performance improvements\n"
        "4. Best practices\n"
        "5. Alternative approaches (if applicable)\n"
        "6. Direct code fixes if needed\n\n"
        "Make your response actionable and clear. If you're suggesting code changes, show the before and after code.";

    return sendMessage(prompt, "Code Analysis Request");
}

std::string GeminiService::fixErrors(const std::string& errorLog, const std::string& code) {
    std::string prompt = 
        "You are a coding assistant helping to fix errors in a VS Code project.\n\n"
        "Error Log:\n" + errorLog + "\n\n"
        "Current Code:\n```\n" + (code.empty() ? "No code provided" : code) + "\n```\n\n"
        "Please analyze the error and provide:\n"
        "1. Root cause analysis\n"
        "2. Step-by-step fix instructions\n"
        "3. The corrected code (if applicable)\n"
        "4. Prevention tips for the future\n\n"
        "Be specific and provide actionable solutions.";

    return sendMessage(prompt, "Error Fix Request");
}

void GeminiService::clearHistory() {
    chatHistory.clear();
}