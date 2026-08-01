#pragma once
#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <atomic>

class WebServer {
private:
    int port;
    std::atomic<bool> m_isRunning;
    std::unique_ptr<std::thread> serverThread;

    std::function<std::string(const std::string&, const std::string&)> m_chatHandler;
    std::function<std::string(const std::string&, const std::string&)> m_analyzeHandler;
    std::function<std::string(const std::string&, const std::string&)> m_fixErrorsHandler;

    void runServer();
    std::string handleRequest(const std::string& request);

public:
    WebServer(int port);
    ~WebServer();

    bool start();
    void stop();
    bool isRunning() const { return m_isRunning.load(); }

    void setChatHandler(std::function<std::string(const std::string&, const std::string&)> handler);
    void setAnalyzeHandler(std::function<std::string(const std::string&, const std::string&)> handler);
    void setFixErrorsHandler(std::function<std::string(const std::string&, const std::string&)> handler);
};