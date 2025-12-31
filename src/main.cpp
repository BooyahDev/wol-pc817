#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <vector>
#include "config.h"
#include "web_ui.h"

// Webサーバインスタンス
AsyncWebServer server(WEB_SERVER_PORT);

// 各PCの状態を保存（trueならON状態と仮定）
bool pcStates[4] = {false, false, false, false};

// フォトカプラ初期化
void initPhotocouplers() {
    for (int i = 0; i < NUM_PHOTOCOUPLERS; i++) {
        pinMode(PHOTOCOUPLER_PINS[i], OUTPUT);
        digitalWrite(PHOTOCOUPLER_PINS[i], LOW);
    }
    Serial.println("Photocouplers initialized");
}

// PC電源ボタンを押す（パルス信号を送る）
void pressPowerButton(int pcIndex) {
    if (pcIndex < 0 || pcIndex >= NUM_PHOTOCOUPLERS) {
        Serial.println("Invalid PC index");
        return;
    }
    
    Serial.printf("Pressing power button for %s (GPIO %d)\n", 
                  PC_NAMES[pcIndex], PHOTOCOUPLER_PINS[pcIndex]);
    
    digitalWrite(PHOTOCOUPLER_PINS[pcIndex], HIGH);
    delay(POWER_PULSE_MS);
    digitalWrite(PHOTOCOUPLER_PINS[pcIndex], LOW);
    
    // 状態を反転（簡易的な状態管理）
    pcStates[pcIndex] = !pcStates[pcIndex];
    
    Serial.printf("%s power button pressed\n", PC_NAMES[pcIndex]);
}

// PC電源ボタンを長押し（強制シャットダウン用）
void longPressPowerButton(int pcIndex) {
    if (pcIndex < 0 || pcIndex >= NUM_PHOTOCOUPLERS) {
        Serial.println("Invalid PC index");
        return;
    }
    
    Serial.printf("Long pressing power button for %s (GPIO %d) - %dms\n", 
                  PC_NAMES[pcIndex], PHOTOCOUPLER_PINS[pcIndex], POWER_LONG_PRESS_MS);
    
    digitalWrite(PHOTOCOUPLER_PINS[pcIndex], HIGH);
    delay(POWER_LONG_PRESS_MS);
    digitalWrite(PHOTOCOUPLER_PINS[pcIndex], LOW);
    
    // 強制シャットダウンなので状態はOFF
    pcStates[pcIndex] = false;
    
    Serial.printf("%s power button long pressed (forced shutdown)\n", PC_NAMES[pcIndex]);
}

// Wi-Fi接続
void connectWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connection failed!");
    }
}

// HTMLページを返す（共通ビルダー）
String getIndexHTML() {
    std::vector<std::string> names;
    for (int i = 0; i < NUM_PHOTOCOUPLERS; i++) {
        names.emplace_back(PC_NAMES[i]);
    }
    std::string html = buildWebUI(names, "ESP32", "実機", NUM_PHOTOCOUPLERS);
    return String(html.c_str());
}

// Webサーバのルート設定
void setupWebServer() {
    // ルートページ
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", getIndexHTML());
    });
    
    // API: システム情報取得
    server.on("/api/info", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
        json += "\"numPCs\":" + String(NUM_PHOTOCOUPLERS) + ",";
        json += "\"pcNames\":[";
        for (int i = 0; i < NUM_PHOTOCOUPLERS; i++) {
            json += "\"" + String(PC_NAMES[i]) + "\"";
            if (i < NUM_PHOTOCOUPLERS - 1) json += ",";
        }
        json += "]}";
        request->send(200, "application/json", json);
    });
    
    // API: PC状態取得
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{\"states\":[";
        for (int i = 0; i < NUM_PHOTOCOUPLERS; i++) {
            json += pcStates[i] ? "true" : "false";
            if (i < NUM_PHOTOCOUPLERS - 1) json += ",";
        }
        json += "]}";
        request->send(200, "application/json", json);
    });
    
    // API: 電源操作
    server.on("^\\/api\\/power\\/([0-9]+)$", HTTP_POST, [](AsyncWebServerRequest *request) {
        String pcIndexStr = request->pathArg(0);
        int pcIndex = pcIndexStr.toInt();
        
        if (pcIndex >= 0 && pcIndex < NUM_PHOTOCOUPLERS) {
            pressPowerButton(pcIndex);
            
            String json = "{";
            json += "\"success\":true,";
            json += "\"message\":\"" + String(PC_NAMES[pcIndex]) + " power button pressed\",";
            json += "\"pcIndex\":" + String(pcIndex) + ",";
            json += "\"newState\":" + String(pcStates[pcIndex] ? "true" : "false");
            json += "}";
            
            request->send(200, "application/json", json);
        } else {
            String json = "{\"success\":false,\"message\":\"Invalid PC index\"}";
            request->send(400, "application/json", json);
        }
    });
    
    // API: 電源長押し操作（強制シャットダウン）
    server.on("^\\/api\\/longpress\\/([0-9]+)$", HTTP_POST, [](AsyncWebServerRequest *request) {
        String pcIndexStr = request->pathArg(0);
        int pcIndex = pcIndexStr.toInt();
        
        if (pcIndex >= 0 && pcIndex < NUM_PHOTOCOUPLERS) {
            longPressPowerButton(pcIndex);
            
            String json = "{";
            json += "\"success\":true,";
            json += "\"message\":\"" + String(PC_NAMES[pcIndex]) + " power button long pressed (forced shutdown)\",";
            json += "\"pcIndex\":" + String(pcIndex) + ",";
            json += "\"duration\":" + String(POWER_LONG_PRESS_MS);
            json += "}";
            
            request->send(200, "application/json", json);
        } else {
            String json = "{\"success\":false,\"message\":\"Invalid PC index\"}";
            request->send(400, "application/json", json);
        }
    });
    
    // 404エラー
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "application/json", "{\"error\":\"Not found\"}");
    });
    
    server.begin();
    Serial.println("Web server started");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=================================");
    Serial.println("PC Power Control with ESP32");
    Serial.println("=================================\n");
    
    // フォトカプラ初期化
    initPhotocouplers();
    
    // Wi-Fi接続
    connectWiFi();
    
    // Webサーバ起動
    if (WiFi.status() == WL_CONNECTED) {
        setupWebServer();
        Serial.println("\nSystem ready!");
        Serial.print("Access: http://");
        Serial.println(WiFi.localIP());
    }
}

void loop() {
    // メインループは空（AsyncWebServerが非同期で動作）
    delay(100);
}
