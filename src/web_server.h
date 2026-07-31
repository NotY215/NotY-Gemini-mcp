#pragma once
#include <string>
#include <functional>
#include <thread>
#include <memory>

class WebServer {
private:
    int port;
    bool isRunning;
    std::unique_ptr<std::thread> serverThread;
    
    std::function<std::string(const std::string&, const std::string&)> chatHandler;
    std::function<std::string(const std::string&, const std::string&)> analyzeHandler;
    std::function<std::string(const std::string&, const std::string&)> fixErrorsHandler;

    void runServer();
    void handleRequest(const std::string& request, std::string& response);

public:
    WebServer(int port);
    ~WebServer();

    bool start();
    void stop();
    bool isRunning() const { return isRunning; }

    void onChat(std::function<std::string(const std::string&, const std::string&)> handler);
    void onAnalyze(std::function<std::string(const std::string&, const std::string&)> handler);
    void onFixErrors(std::function<std::string(const std::string&, const std::string&)> handler);
};