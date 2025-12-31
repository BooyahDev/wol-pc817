# PC Power Control with ESP32 & Photocoupler

ESP32とフォトカプラ（PC817等）を使用して、複数台のPCの電源を遠隔制御するシステムです。

## 機能

- Wi-Fi経由でWebサーバにアクセス
- REST APIでPC電源のON/OFF制御
- 複数台のPC（フォトカプラ）に対応
- Webブラウザからの簡単操作

## 必要な部品

- ESP32開発ボード
- フォトカプラ（PC817など） × PC台数分
- 抵抗（330Ω程度） × フォトカプラ数
- ジャンパーワイヤー
- ブレッドボード（任意）

## 回路図

```
ESP32 GPIO → 抵抗(330Ω) → フォトカプラLED(+) → GND
                           フォトカプラ出力 → PCマザーボード電源スイッチ端子
```

### フォトカプラ（PC817）のピン配置

```
    PC817
   ┌─────┐
1  │●    │ 4
   │     │
2  │     │ 3
   └─────┘

1: アノード(+)   → 抵抗 → ESP32 GPIO
2: カソード(-)   → GND
3: コレクタ      → PCマザーボード PWR_SW +
4: エミッタ      → PCマザーボード PWR_SW -
```

## セットアップ

### 1. 設定ファイルの作成

`src/config.h.example` を `src/config.h` にコピーして、自分の環境に合わせて編集してください：

```bash
cp src/config.h.example src/config.h
```

設定内容：
- Wi-Fi SSID とパスワード
- フォトカプラ接続GPIOピン番号

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
  "photocouplersCount": 4,
  "pins": [16, 17, 18, 19],
  "ip": "192.168.1.100",
  "uptime": 3600
}
```

## マザーボードへの接続

1. PCマザーボードの電源スイッチコネクタ（通常 `PWR_SW` または `POWER SW`）を確認
2. フォトカプラの出力側（コレクタとエミッタ）をこの端子に接続
3. 極性は通常気にしなくて良いが、動作しない場合は逆にしてみる

**注意**: マザーボードの電源端子に接続する際は、必ずPCの電源を切り、電源ケーブルを抜いてから作業してください。

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
