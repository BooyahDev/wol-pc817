# PC Power Control with ESP32 & Photocoupler

ESP32とフォトカプラ（PC817等）を使用して、複数台のPCの電源を遠隔制御するシステムです。

## 機能

- Wi-Fi経由でWebサーバにアクセス
- REST APIでPC電源のON/OFF制御
- 複数台のPC（フォトカプラ）に対応（PC1〜PC9）
- LED状態の読み取り機能（LED1〜LED9）
- Webブラウザからの簡単操作

## 必要な部品

- ESP32開発ボード
- フォトカプラ（PC817など） × PC台数分（最大9台）
- 抵抗（330Ω程度） × フォトカプラ数
- ジャンパーワイヤー
- ブレッドボード（任意）
- LED状態確認用の配線（オプション）

## 回路図

### 電源制御用（OUTPUT）- フォトカプラによる電源スイッチ制御
```
ESP32 GPIO(PC1-PC9) → 抵抗(330Ω) → フォトカプラLED(+) → GND
                                    フォトカプラ出力 → PCマザーボード電源スイッチ端子
```

### LED状態読み取り用（INPUT）- フォトカプラによる絶縁入力
```
PCのLED（電源LED等）
    (+) → 抵抗(330Ω〜1kΩ) → フォトカプラLED(+)
    (-) → フォトカプラLED(-)
                              フォトカプラ出力 → ESP32 GPIO(LED1-LED9)
```

### フォトカプラ（PC817）のピン配置と結線

#### OUTPUT用（電源制御）

```
    PC817                        接続先
   ┌─────┐
1  │●    │ 4                1: アノード(+)   ← 抵抗(330Ω) ← ESP32 GPIO
   │     │                  2: カソード(-)   → GND
2  │     │ 3                3: コレクタ      → PCマザーボード PWR_SW +
   └─────┘                  4: エミッタ      → PCマザーボード PWR_SW -

ESP32がHIGHを出力 → フォトカプラLED点灯 → 出力側導通 → 電源スイッチON
```

#### INPUT用（LED状態読み取り）

```
    PC817                        接続先
   ┌─────┐
1  │●    │ 4                1: アノード(+)   ← 抵抗(330Ω〜1kΩ) ← PCのLED(+)
   │     │                  2: カソード(-)   → PCのLED(-) または GND
2  │     │ 3                3: コレクタ      → ESP32 3.3V (またはプルアップ抵抗経由)
   └─────┘                  4: エミッタ      → ESP32 GPIO(INPUT)

PCのLED点灯 → フォトカプラLED点灯 → 出力側導通 → ESP32がHIGHを検出
```

**INPUT用の推奨接続方法：**
- コレクタを3.3Vまたは10kΩプルアップ抵抗経由で3.3Vに接続
- エミッタをESP32のGPIOピンに接続
- GPIOピンとGND間に10kΩプルダウン抵抗を接続（オプション）
- LED点灯時にGPIOがHIGH、消灯時にLOWとなる

## セットアップ

### 1. 設定ファイルの作成

`src/config.h.example` を `src/config.h` にコピーして、自分の環境に合わせて編集してください：

```bash
cp src/config.h.example src/config.h
```

設定内容：
- Wi-Fi SSID とパスワード
- フォトカプラ接続GPIOピン番号（PC1〜PC9）
- LED読み取り用GPIOピン番号（LED1〜LED9）

#### デフォルトのピン配列

**電源制御用（OUTPUT）:**
- PC1: GPIO32, PC2: GPIO33, PC3: GPIO25, PC4: GPIO26, PC5: GPIO27
- PC6: GPIO14, PC7: GPIO12, PC8: GPIO13, PC9: GPIO23

**LED読み取り用（INPUT）:**
- LED1: GPIO22, LED2: GPIO21, LED3: GPIO19, LED4: GPIO18, LED5: GPIO5
- LED6: GPIO17, LED7: GPIO16, LED8: GPIO4, LED9: GPIO2

### 2. ライブラリのインストール

PlatformIOが自動的に必要なライブラリをインストールします：
- ESP Async WebServer
- AsyncTCP

### 3. ビルド＆アップロード

```bash
# ビルド
pio run

# ESP32にアップロード
pio run --target upload

# シリアルモニタでログ確認
pio device monitor
```

### 4. IPアドレスの確認

シリアルモニタでESP32が取得したIPアドレスを確認してください。

## システム構成

本システムは以下の2つの機能を持ちます：

1. **電源制御機能（OUTPUT）**: PC1〜PC9用のフォトカプラで電源ボタンを制御
2. **LED監視機能（INPUT）**: LED1〜LED9のGPIOピンでLEDの点灯状態を読み取り

### 動作

- 電源制御: Webインターフェースまたは REST API で指定のPCに電源パルスを送信
- LED監視: 500msごとにLED状態を自動読み取り、APIで取得可能
- 長押し機能: 強制シャットダウン用の長押し（5秒）も対応

## 使い方

### Webブラウザから操作

ESP32のIPアドレスにブラウザでアクセス：
```
http://<ESP32のIPアドレス>/
```

各PCのボタンをクリックすると電源パルスが送信されます。

### REST API

#### 個別のPC制御

```bash
# PC 0の電源をON/OFF
curl http://<ESP32のIP>/api/power/0

# PC 1の電源をON/OFF
curl http://<ESP32のIP>/api/power/1
```

#### 全PC制御

```bash
# すべてのPCに電源パルスを送信
curl http://<ESP32のIP>/api/power/all
```

#### ステータス確認

```bash
curl http://<ESP32のIP>/api/status
```

レスポンス例：
```json
{
  "pcStates": [false, false, true, false, false, false, false, false, false],
  "ledStates": [false, false, true, false, false, true, false, false, false]
}
```

#### システム情報確認

```bash
curl http://<ESP32のIP>/api/info
```

レスポンス例：
```json
{
  "ip": "192.168.1.100",
  "numPCs": 9,
  "numLEDs": 9,
  "pcNames": ["proxmox001", "proxmox002", "proxmox003", "proxmox004", "proxmox005", "proxmox006", "proxmox007", "PC08", "PC09"],
  "ledNames": ["LED1", "LED2", "LED3", "LED4", "LED5", "LED6", "LED7", "LED8", "LED9"]
}
```

## ハードウェア接続方法

### 電源制御用フォトカプラの接続

1. **マザーボード側の確認**
   - PCマザーボードの電源スイッチコネクタ（通常 `PWR_SW` または `POWER SW`）を確認
   - フォトカプラの出力側（コレクタとエミッタ）をこの端子に接続
   - 極性は通常気にしなくて良いが、動作しない場合は逆にしてみる

2. **ESP32側の接続**
   - ESP32のGPIO → 抵抗（330Ω） → フォトカプラのアノード（ピン1）
   - フォトカプラのカソード（ピン2） → GND

### LED読み取り用フォトカプラの接続

1. **PCのLED側の接続**
   - PCマザーボードの電源LEDまたはHDD LEDコネクタ（`POWER LED` または `PWR_LED`）を確認
   - LED(+) → 抵抗（330Ω〜1kΩ） → フォトカプラのアノード（ピン1）
   - LED(-) → フォトカプラのカソード（ピン2）
   - 既存のLEDと並列に接続する場合は、元のLEDも動作します

2. **ESP32側の接続**
   - フォトカプラのコレクタ（ピン3） → ESP32の3.3V（またはプルアップ抵抗10kΩ経由で3.3V）
   - フォトカプラのエミッタ（ピン4） → ESP32のGPIO（INPUT設定）
   - オプション: GPIO → プルダウン抵抗10kΩ → GND（より安定した動作のため）

**重要な注意事項**:
- マザーボードの端子に接続する際は、必ずPCの電源を切り、電源ケーブルを抜いてから作業してください
- フォトカプラを使用することで、PC側とESP32側が電気的に絶縁され、安全に接続できます
- 抵抗値は使用するフォトカプラとLEDの仕様に応じて調整してください

## トラブルシューティング

### Wi-Fiに接続できない
- `config.h` のSSIDとパスワードを確認
- ルータが2.4GHz帯をサポートしているか確認（ESP32は5GHz非対応）

### 電源が入らない
- フォトカプラの配線を確認
- 抵抗値を確認（330Ω〜1kΩ程度）
- シリアルモニタでパルスが送信されているか確認
- マザーボードの電源スイッチ端子が正しいか確認

### ブラウザでアクセスできない
- ESP32とPCが同じネットワークにいるか確認
- ファイアウォール設定を確認
- シリアルモニタでIPアドレスを再確認

## ライセンス

MIT License

## 参考

- PC817データシート: フォトカプラの仕様
- ESP32 Arduino Core: https://github.com/espressif/arduino-esp32
- ESPAsyncWebServer: https://github.com/me-no-dev/ESPAsyncWebServer
