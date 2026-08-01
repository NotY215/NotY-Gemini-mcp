#include "web_server.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

WebServer::WebServer(int port) : port(port), m_isRunning(false) {}

WebServer::~WebServer() {
    stop();
}

bool WebServer::start() {
    if (m_isRunning.load()) {
        return true;
    }

    serverThread = std::make_unique<std::thread>(&WebServer::runServer, this);
    m_isRunning = true;
    return true;
}

void WebServer::stop() {
    m_isRunning = false;
    if (serverThread && serverThread->joinable()) {
        serverThread->join();
    }
}

void WebServer::runServer() {
    try {
        net::io_context ioc{1};
        tcp::acceptor acceptor{ioc, tcp::endpoint(tcp::v4(), static_cast<unsigned short>(port))};

        while (m_isRunning.load()) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);

            beast::flat_buffer buffer;
            http::request<http::string_body> req;
            http::read(socket, buffer, req);

            http::response<http::string_body> res;
            std::string response;

            if (req.method() == http::verb::post) {
                response = handleRequest(req.body());

                res.result(http::status::ok);
                res.set(http::field::content_type, "application/json");
                res.body() = response;
                res.prepare_payload();
            } else {
                // Get the target path as string
                std::string path = std::string(req.target());
                if (path == "/") path = "/index.html";

                size_t queryPos = path.find('?');
                if (queryPos != std::string::npos) {
                    path = path.substr(0, queryPos);
                }

                std::string filePath = "web" + path;
                std::ifstream file(filePath, std::ios::binary);
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

std::string WebServer::handleRequest(const std::string& request) {
    try {
        auto json = nlohmann::json::parse(request);
        std::string type = json["type"];
        nlohmann::json result;

        if (type == "chat" && m_chatHandler) {
            std::string message = json["message"];
            std::string context = json.value("context", "");
            std::string response_text = m_chatHandler(message, context);
            result["response"] = response_text;
            result["success"] = true;
        }
        else if (type == "analyze" && m_analyzeHandler) {
            std::string code = json["code"];
            std::string question = json["question"];
            std::string response_text = m_analyzeHandler(code, question);
            result["response"] = response_text;
            result["success"] = true;
        }
        else if (type == "fix-errors" && m_fixErrorsHandler) {
            std::string errorLog = json["errorLog"];
            std::string code = json["code"];
            std::string response_text = m_fixErrorsHandler(errorLog, code);
            result["response"] = response_text;
            result["success"] = true;
        }
        else {
            result["success"] = false;
            result["error"] = "Unknown request type";
        }

        return result.dump();
    }
    catch (const std::exception& e) {
        nlohmann::json error;
        error["success"] = false;
        error["error"] = e.what();
        return error.dump();
    }
}

void WebServer::setChatHandler(std::function<std::string(const std::string&, const std::string&)> handler) {
    m_chatHandler = handler;
}

void WebServer::setAnalyzeHandler(std::function<std::string(const std::string&, const std::string&)> handler) {
    m_analyzeHandler = handler;
}

void WebServer::setFixErrorsHandler(std::function<std::string(const std::string&, const std::string&)> handler) {
    m_fixErrorsHandler = handler;
}