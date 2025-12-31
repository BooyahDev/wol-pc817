# ESP32 HTTPサーバ シミュレータ

macOS (ARM64) 上でESP32のHTTPサーバをDockerでシミュレーションする環境です。

## 🎯 機能

- ESP32のWebサーバ機能を完全再現
- PC電源制御のシミュレーション（GPIO操作をログ出力）
- レスポンシブなWebインターフェース
- リアルタイムなPC状態管理
- 通常の電源操作と強制シャットダウン（長押し）に対応

## 🚀 使い方

### 1. Dockerイメージのビルドと起動

```bash
# Docker Composeで起動
docker-compose up --build

# またはDockerコマンドで起動
docker build -t esp32-simulator .
docker run -p 8080:80 esp32-simulator
```

### 2. Webインターフェースにアクセス

ブラウザで以下のURLを開きます：

```
http://localhost:8080
```

### 3. 操作方法

- **Power Toggle**: PC電源をON/OFFします（500msのパルス信号をシミュレート）
- **Force Shutdown**: 5秒間の長押しで強制シャットダウン

## 📡 API エンドポイント

### GET /
Webインターフェースを表示

### GET /api/info
システム情報を取得
```json
{
  "ip": "localhost",
  "numPCs": 4,
  "pcNames": ["PC-01", "PC-02", "PC-03", "PC-04"]
}
```

### GET /api/status
PC状態を取得
```json
{
  "states": [false, false, false, false]
}
```

### POST /api/power/{pcIndex}
PC電源をトグル（0-3）
```json
{
  "success": true,
  "message": "PC-01 power button pressed",
  "pcIndex": 0,
  "newState": true
}
```

### POST /api/longpress/{pcIndex}
PC電源を長押し（強制シャットダウン）
```json
{
  "success": true,
  "message": "PC-01 power button long pressed (forced shutdown)",
  "pcIndex": 0,
  "duration": 5000
}
```

## 🔍 ログの確認

コンテナのログをリアルタイムで確認：

```bash
docker-compose logs -f
```

または：

```bash
docker logs -f esp32-simulator
```

### ログ出力例

```
=================================
ESP32 HTTP Server Simulator
Platform: macOS ARM64 (Docker)
=================================

[INFO] Photocouplers initialized (simulated)
[INFO] Web server started on port 80
[INFO] Access: http://localhost:80

[INFO] System ready! Waiting for connections...

[HTTP] POST /api/power/0
[INFO] Pressing power button for PC-01 (GPIO 25)
[GPIO] Pin 25 -> HIGH
[GPIO] Pin 25 -> LOW
[INFO] PC-01 power button pressed. New state: ON
```

## 🛠️ 開発・デバッグ

### コンテナに入る

```bash
docker exec -it esp32-simulator /bin/bash
```

### コンテナを停止

```bash
docker-compose down
```

### イメージを再ビルド

```bash
docker-compose up --build --force-recreate
```

## 📝 実機との違い

| 項目 | 実機（ESP32） | シミュレータ |
|------|--------------|-------------|
| GPIO操作 | 実際にピンを制御 | ログに出力 |
| WiFi | 実際のネットワーク接続 | localhost |
| 遅延 | ハードウェア依存 | シミュレート |
| IPアドレス | DHCP等で取得 | localhost固定 |

## 🔧 カスタマイズ

### ポート番号の変更

[docker-compose.yml](docker-compose.yml) の `ports` セクションを編集：

```yaml
ports:
  - "9000:80"  # ホストの9000番ポートにマッピング
```

### PC台数や名前の変更

[simulator/esp32_simulator.cpp](simulator/esp32_simulator.cpp) の定数を編集：

```cpp
const int NUM_PHOTOCOUPLERS = 4;
const char* PC_NAMES[] = {"PC-01", "PC-02", "PC-03", "PC-04"};
```

## 🎓 利点

- **実機不要**: ESP32ハードウェアなしで開発・テスト可能
- **高速**: ビルドとデプロイが高速
- **デバッグ容易**: ログがわかりやすく、デバッグが簡単
- **クロスプラットフォーム**: ARM64/AMD64両対応
- **安全**: ハードウェアを壊す心配なし

## 📦 システム要件

- Docker Desktop for Mac (ARM64)
- macOS 11.0以降
- 空きメモリ: 512MB以上
