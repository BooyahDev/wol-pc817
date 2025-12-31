#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstring>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "web_ui.h"

// ESP32のconfig.hをシミュレート
const int NUM_PHOTOCOUPLERS = 4;
const char* PC_NAMES[] = {"PC-01", "PC-02", "PC-03", "PC-04"};
const int PHOTOCOUPLER_PINS[] = {25, 26, 27, 14};
const int POWER_PULSE_MS = 500;
const int POWER_LONG_PRESS_MS = 5000;
const int WEB_SERVER_PORT = 80;

// HTTPレスポンスヘッダーを生成
std::string createHttpResponse(int statusCode, const std::string& contentType, const std::string& body) {
    std::ostringstream response;
    std::string statusText = (statusCode == 200) ? "OK" : 
                            (statusCode == 404) ? "Not Found" : "Bad Request";
    
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "\r\n";
    response << body;
    
    return response.str();
}

// 電源ボタンを押す（シミュレート）
void pressPowerButton(int pcIndex) {
    std::cout << "[INFO] Pressing power button for " << PC_NAMES[pcIndex] 
              << " (GPIO " << PHOTOCOUPLER_PINS[pcIndex] << ")" << std::endl;
    
    // GPIOをHIGHに設定（シミュレート）
    std::cout << "[GPIO] Pin " << PHOTOCOUPLER_PINS[pcIndex] << " -> HIGH" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(POWER_PULSE_MS));
    
    // GPIOをLOWに設定（シミュレート）
    std::cout << "[GPIO] Pin " << PHOTOCOUPLER_PINS[pcIndex] << " -> LOW" << std::endl;
    
    std::cout << "[INFO] " << PC_NAMES[pcIndex] << " power button pressed." << std::endl;
}

// 電源ボタンを長押し（シミュレート）
void longPressPowerButton(int pcIndex) {
    std::cout << "[INFO] Long pressing power button for " << PC_NAMES[pcIndex]
              << " (GPIO " << PHOTOCOUPLER_PINS[pcIndex] << ") - "
              << POWER_LONG_PRESS_MS << "ms" << std::endl;

    std::cout << "[GPIO] Pin " << PHOTOCOUPLER_PINS[pcIndex] << " -> HIGH" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(POWER_LONG_PRESS_MS));
    std::cout << "[GPIO] Pin " << PHOTOCOUPLER_PINS[pcIndex] << " -> LOW" << std::endl;

    std::cout << "[INFO] " << PC_NAMES[pcIndex] << " power button long pressed (forced shutdown)" << std::endl;
}

// HTMLページを生成（共通ビルダー使用）
std::string getIndexHTML() {
    std::vector<std::string> names;
    for (int i = 0; i < NUM_PHOTOCOUPLERS; i++) {
        names.emplace_back(PC_NAMES[i]);
    }
    return buildWebUI(names, "macOS ARM64", "ESP32シミュレータ", NUM_PHOTOCOUPLERS);
}

// HTTPリクエストを処理
void handleRequest(int clientSocket, const std::string& request) {
    std::istringstream iss(request);
    std::string method, path, version;
    iss >> method >> path >> version;
    
    std::cout << "[HTTP] " << method << " " << path << std::endl;
    
    std::string response;
    
    if (method == "GET" && path == "/") {
        response = createHttpResponse(200, "text/html; charset=utf-8", getIndexHTML());
    }
    else if (method == "GET" && path == "/api/info") {
        std::ostringstream json;
        json << "{\"ip\":\"localhost\",\"numPCs\":" << NUM_PHOTOCOUPLERS << ",\"pcNames\":[";
        for (int i = 0; i < NUM_PHOTOCOUPLERS; i++) {
            json << "\"" << PC_NAMES[i] << "\"";
            if (i < NUM_PHOTOCOUPLERS - 1) json << ",";
        }
        json << "]}";
        response = createHttpResponse(200, "application/json", json.str());
    }
    else if (method == "GET" && path == "/api/status") {
        // 状態を保持しないため常にfalseを返す
        std::ostringstream json;
        json << "{\"states\":[";
        for (int i = 0; i < NUM_PHOTOCOUPLERS; i++) {
            json << "false";
            if (i < NUM_PHOTOCOUPLERS - 1) json << ",";
        }
        json << "]}";
        response = createHttpResponse(200, "application/json", json.str());
    }
    else if (method == "POST" && path.find("/api/power/") == 0) {
        int pcIndex = std::stoi(path.substr(11));
        if (pcIndex >= 0 && pcIndex < NUM_PHOTOCOUPLERS) {
            pressPowerButton(pcIndex);
            std::ostringstream json;
              json << "{\"success\":true,\"message\":\"" << PC_NAMES[pcIndex] 
                  << " の電源ボタンを押しました\",\"pcIndex\":" << pcIndex << "}";
            response = createHttpResponse(200, "application/json", json.str());
        } else {
            response = createHttpResponse(400, "application/json", "{\"success\":false,\"message\":\"Invalid PC index\"}");
        }
    }
    else if (method == "POST" && path.find("/api/longpress/") == 0) {
        int pcIndex = std::stoi(path.substr(15));
        if (pcIndex >= 0 && pcIndex < NUM_PHOTOCOUPLERS) {
            longPressPowerButton(pcIndex);
            std::ostringstream json;
              json << "{\"success\":true,\"message\":\"" << PC_NAMES[pcIndex] 
                  << " の電源ボタンを長押ししました (強制シャットダウン)\",\"pcIndex\":" << pcIndex 
                  << ",\"duration\":" << POWER_LONG_PRESS_MS << "}";
            response = createHttpResponse(200, "application/json", json.str());
        } else {
            response = createHttpResponse(400, "application/json", "{\"success\":false,\"message\":\"Invalid PC index\"}");
        }
    }
    else {
        response = createHttpResponse(404, "application/json", "{\"error\":\"Not found\"}");
    }
    
    send(clientSocket, response.c_str(), response.length(), 0);
}

int main() {
    std::cout << "\n=================================" << std::endl;
    std::cout << "ESP32 HTTP Server Simulator" << std::endl;
    std::cout << "Platform: macOS ARM64 (Docker)" << std::endl;
    std::cout << "=================================\n" << std::endl;
    
    // ソケット作成
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }
    
    // SO_REUSEADDRオプションを設定
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // アドレス設定
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(WEB_SERVER_PORT);
    
    // バインド
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        close(serverSocket);
        return 1;
    }
    
    // リッスン
    if (listen(serverSocket, 10) < 0) {
        std::cerr << "Error listening" << std::endl;
        close(serverSocket);
        return 1;
    }
    
    std::cout << "[INFO] Photocouplers initialized (simulated)" << std::endl;
    std::cout << "[INFO] Web server started on port " << WEB_SERVER_PORT << std::endl;
    std::cout << "[INFO] Access: http://localhost:" << WEB_SERVER_PORT << std::endl;
    std::cout << "\n[INFO] System ready! Waiting for connections...\n" << std::endl;
    
    // 接続を受け付ける
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }
        
        // リクエストを読み取る
        char buffer[4096] = {0};
        read(clientSocket, buffer, sizeof(buffer));
        std::string request(buffer);
        
        // 別スレッドでリクエストを処理
        std::thread([clientSocket, request]() {
            handleRequest(clientSocket, request);
            close(clientSocket);
        }).detach();
    }
    
    close(serverSocket);
    return 0;
}
