#pragma once
#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <atomic>

class WebServer {
private:
    int port;
    std::atomic<bool> isRunning;
    std::unique_ptr<std::thread> serverThread;

    std::function<std::string(const std::string&, const std::string&)> chatHandler;
    std::function<std::string(const std::string&, const std::string&)> analyzeHandler;
    std::function<std::string(const std::string&, const std::string&)> fixErrorsHandler;

    void runServer();
    std::string handleRequest(const std::string& request);

public:
    WebServer(int port);
    ~WebServer();

    bool start();
    void stop();
    bool isRunning() const { return isRunning.load(); }

    void onChat(std::function<std::string(const std::string&, const std::string&)> handler);
    void onAnalyze(std::function<std::string(const std::string&, const std::string&)> handler);
    void onFixErrors(std::function<std::string(const std::string&, const std::string&)> handler);
};