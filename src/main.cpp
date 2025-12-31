#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "config.h"

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

// HTMLページを返す
String getIndexHTML() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>PC Power Control</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
        }
        h1 {
            color: white;
            text-align: center;
            margin-bottom: 30px;
            font-size: 2.5em;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .pc-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        .pc-card {
            background: white;
            border-radius: 15px;
            padding: 25px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            transition: transform 0.3s ease;
        }
        .pc-card:hover {
            transform: translateY(-5px);
        }
        .pc-name {
            font-size: 1.5em;
            font-weight: bold;
            color: #333;
            margin-bottom: 15px;
        }
        .pc-status {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            font-size: 0.9em;
            margin-bottom: 15px;
            font-weight: 500;
        }
        .status-unknown {
            background: #e0e0e0;
            color: #666;
        }
        .status-on {
            background: #4caf50;
            color: white;
        }
        .status-off {
            background: #f44336;
            color: white;
        }
        .button {
            width: 100%;
            padding: 15px;
            border: none;
            border-radius: 10px;
            font-size: 1.1em;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s ease;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        .btn-power {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }
        .btn-power:hover {
            transform: scale(1.05);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }
        .btn-power:active {
            transform: scale(0.95);
        }
        .btn-longpress {
            background: linear-gradient(135deg, #f44336 0%, #e91e63 100%);
            color: white;
            margin-top: 10px;
        }
        .btn-longpress:hover {
            transform: scale(1.05);
            box-shadow: 0 5px 15px rgba(244, 67, 54, 0.4);
        }
        .btn-longpress:active {
            transform: scale(0.95);
        }
        .info-box {
            background: white;
            border-radius: 15px;
            padding: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            color: #333;
        }
        .info-box h2 {
            color: #667eea;
            margin-bottom: 10px;
        }
        .message {
            margin-top: 10px;
            padding: 10px;
            border-radius: 5px;
            display: none;
        }
        .message.success {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .message.error {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        @media (max-width: 600px) {
            h1 {
                font-size: 2em;
            }
            .pc-grid {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🖥️ PC Power Control</h1>
        
        <div class="pc-grid" id="pcGrid">
            <!-- PC cards will be inserted here -->
        </div>
        
        <div class="info-box">
            <h2>📡 System Information</h2>
            <p><strong>ESP32 IP:</strong> <span id="ipAddress">Loading...</span></p>
            <p><strong>Connected PCs:</strong> <span id="pcCount">)rawliteral" + 
                String(NUM_PHOTOCOUPLERS) + R"rawliteral(</span></p>
            <div id="message" class="message"></div>
        </div>
    </div>

    <script>
        const pcNames = [
)rawliteral";

    // PC名を追加
    for (int i = 0; i < NUM_PHOTOCOUPLERS; i++) {
        html += "            \"" + String(PC_NAMES[i]) + "\"";
        if (i < NUM_PHOTOCOUPLERS - 1) html += ",";
        html += "\n";
    }
    
    html += R"rawliteral(
        ];

        const pcStates = [
)rawliteral";

    // PC状態を追加
    for (int i = 0; i < NUM_PHOTOCOUPLERS; i++) {
        html += "            " + String(pcStates[i] ? "true" : "false");
        if (i < NUM_PHOTOCOUPLERS - 1) html += ",";
        html += "\n";
    }

    html += R"rawliteral(
        ];

        function showMessage(text, isSuccess) {
            const msg = document.getElementById('message');
            msg.textContent = text;
            msg.className = 'message ' + (isSuccess ? 'success' : 'error');
            msg.style.display = 'block';
            setTimeout(() => {
                msg.style.display = 'none';
            }, 3000);
        }

        async function togglePower(pcIndex) {
            try {
                const response = await fetch(`/api/power/${pcIndex}`, {
                    method: 'POST'
                });
                
                const data = await response.json();
                
                if (response.ok) {
                    showMessage(data.message, true);
                    // 状態を更新
                    pcStates[pcIndex] = !pcStates[pcIndex];
                    updatePCCards();
                } else {
                    showMessage('Error: ' + data.message, false);
                }
            } catch (error) {
                showMessage('Connection error: ' + error, false);
            }
        }

        async function longPressPower(pcIndex) {
            if (!confirm(`${pcNames[pcIndex]} を強制シャットダウンしますか？\n電源ボタンを5秒間長押しします。`)) {
                return;
            }
            
            try {
                showMessage(`${pcNames[pcIndex]} の電源ボタンを長押ししています...`, true);
                
                const response = await fetch(`/api/longpress/${pcIndex}`, {
                    method: 'POST'
                });
                
                const data = await response.json();
                
                if (response.ok) {
                    showMessage(data.message, true);
                    // 強制シャットダウンなので状態をOFFに
                    pcStates[pcIndex] = false;
                    updatePCCards();
                } else {
                    showMessage('Error: ' + data.message, false);
                }
            } catch (error) {
                showMessage('Connection error: ' + error, false);
            }
        }

        function updatePCCards() {
            const grid = document.getElementById('pcGrid');
            grid.innerHTML = '';
            
            pcNames.forEach((name, index) => {
                const card = document.createElement('div');
                card.className = 'pc-card';
                
                const statusClass = pcStates[index] ? 'status-on' : 'status-off';
                const statusText = pcStates[index] ? 'ON' : 'OFF';
                
                card.innerHTML = `
                    <div class="pc-name">${name}</div>
                    <div class="pc-status ${statusClass}">Status: ${statusText}</div>
                    <button class="button btn-power" onclick="togglePower(${index})">
                        ⚡ Power Toggle
                    </button>
                    <button class="button btn-longpress" onclick="longPressPower(${index})">
                        🔴 Force Shutdown (5s)
                    </button>
                `;
                
                grid.appendChild(card);
            });
        }

        // IPアドレスを取得
        fetch('/api/info')
            .then(response => response.json())
            .then(data => {
                document.getElementById('ipAddress').textContent = data.ip;
            });

        // 初期表示
        updatePCCards();
    </script>
</body>
</html>
)rawliteral";
    
    return html;
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
