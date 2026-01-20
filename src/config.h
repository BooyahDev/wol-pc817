// Wi-Fi設定
#define WIFI_SSID "Booyah-YBB-17676"
#define WIFI_PASSWORD "mizofumi0411"

// Webサーバのポート
#define WEB_SERVER_PORT 80

// フォトカプラ制御用GPIOピンの設定（OUTPUT用）
// PC1, PC2, PC3... の順番で設定
const int PHOTOCOUPLER_PINS[] = {
    32,  // PC1
    33,  // PC2
    25,  // PC3
    26,  // PC4
    27,  // PC5
    14,  // PC6
    12,  // PC7
    13,  // PC8
    23   // PC9
};

// フォトカプラの数
const int NUM_PHOTOCOUPLERS = sizeof(PHOTOCOUPLER_PINS) / sizeof(PHOTOCOUPLER_PINS[0]);

// LED状態読み取り用GPIOピンの設定（INPUT用）
// 旧PC10〜PC18を読み取り専用に変更
const int LED_READ_PINS[] = {
    22,  // LED1 (旧PC10)
    21,  // LED2 (旧PC11)
    19,  // LED3 (旧PC12)
    18,  // LED4 (旧PC13)
    5,   // LED5 (旧PC14)
    17,  // LED6 (旧PC15)
    16,  // LED7 (旧PC16)
    4,   // LED8 (旧PC17)
    2    // LED9 (旧PC18)
};

// LED読み取りピンの数
const int NUM_LED_PINS = sizeof(LED_READ_PINS) / sizeof(LED_READ_PINS[0]);

// PC名称（管理用・OUTPUT用）
const char* PC_NAMES[] = {
    "proxmox001",
    "proxmox002",
    "proxmox003",
    "proxmox004",
    "proxmox005",
    "proxmox006",
    "proxmox007",
    "PC08",
    "PC09"
};

// LED名称（管理用・INPUT用）
const char* LED_NAMES[] = {
    "LED1",
    "LED2",
    "LED3",
    "LED4",
    "LED5",
    "LED6",
    "LED7",
    "LED8",
    "LED9"
};

// パルス幅（ミリ秒） - 電源ボタンを押す時間
#define POWER_PULSE_MS 1500

// 長押し時間（ミリ秒） - 強制シャットダウン用
#define POWER_LONG_PRESS_MS 5000
