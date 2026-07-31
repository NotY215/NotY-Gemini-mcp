#include "web_server.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

WebServer::WebServer(int port) : port(port), isRunning(false) {}

WebServer::~WebServer() {
    stop();
}

bool WebServer::start() {
    if (isRunning) {
        return true;
    }

    serverThread = std::make_unique<std::thread>(&WebServer::runServer, this);
    isRunning = true;
    return true;
}

void WebServer::stop() {
    isRunning = false;
    if (serverThread && serverThread->joinable()) {
        serverThread->join();
    }
}

void WebServer::runServer() {
    try {
        net::io_context ioc{1};
        tcp::acceptor acceptor{ioc, tcp::endpoint(tcp::v4(), port)};

        while (isRunning) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);

            beast::flat_buffer buffer;
            http::request<http::string_body> req;
            http::read(socket, buffer, req);

            http::response<http::string_body> res;
            std::string response;

            // Handle different routes
            if (req.method() == http::verb::post) {
                handleRequest(req.body(), response);
                
                res.result(http::status::ok);
                res.set(http::field::content_type, "application/json");
                res.body() = response;
                res.prepare_payload();
            } else {
                // Serve static files
                std::string path = req.target().to_string();
                if (path == "/") path = "/index.html";
                
                std::string filePath = "web" + path;
                std::ifstream file(filePath);
                if (file.is_open()) {
                    std::string content((std::istreambuf_iterator<char>(file)),
                                       std::istreambuf_iterator<char>());
                    res.result(http::status::ok);
                    res.body() = content;
                    res.prepare_payload();
                } else {
                    res.result(http::status::not_found);
                    res.body() = "File not found";
                    res.prepare_payload();
                }
            }

            http::write(socket, res);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "WebServer error: " << e.what() << std::endl;
    }
}

void WebServer::handleRequest(const std::string& request, std::string& response) {
    try {
        auto json = nlohmann::json::parse(request);
        std::string type = json["type"];
        nlohmann::json result;

        if (type == "chat" && chatHandler) {
            std::string message = json["message"];
            std::string context = json.value("context", "");
            std::string response_text = chatHandler(message, context);
            result["response"] = response_text;
            result["success"] = true;
        }
        else if (type == "analyze" && analyzeHandler) {
            std::string code = json["code"];
            std::string question = json["question"];
            std::string response_text = analyzeHandler(code, question);
            result["response"] = response_text;
            result["success"] = true;
        }
        else if (type == "fix-errors" && fixErrorsHandler) {
            std::string errorLog = json["errorLog"];
            std::string code = json["code"];
            std::string response_text = fixErrorsHandler(errorLog, code);
            result["response"] = response_text;
            result["success"] = true;
        }
        else {
            result["success"] = false;
            result["error"] = "Unknown request type";
        }

        response = result.dump();
    }
    catch (const std::exception& e) {
        nlohmann::json error;
        error["success"] = false;
        error["error"] = e.what();
        response = error.dump();
    }
}

void WebServer::onChat(std::function<std::string(const std::string&, const std::string&)> handler) {
    chatHandler = handler;
}

void WebServer::onAnalyze(std::function<std::string(const std::string&, const std::string&)> handler) {
    analyzeHandler = handler;
}

void WebServer::onFixErrors(std::function<std::string(const std::string&, const std::string&)> handler) {
    fixErrorsHandler = handler;
}