// Wi-Fi設定
#define WIFI_SSID "Buffalo-1F-Kame"
#define WIFI_PASSWORD "0957543350"

// Webサーバのポート
#define WEB_SERVER_PORT 80

// フォトカプラ制御用GPIOピンの設定
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
    23,  // PC9
    22,  // PC10
    21,  // PC11
    19,  // PC12
    18,  // PC13
    5,   // PC14
    17,  // PC15
    16,  // PC16
    4,   // PC17
    2,   // PC18
    15   // PC19
};

// フォトカプラの数
const int NUM_PHOTOCOUPLERS = sizeof(PHOTOCOUPLER_PINS) / sizeof(PHOTOCOUPLER_PINS[0]);

// PC名称（管理用）
const char* PC_NAMES[] = {
    "proxmox001",
    "proxmox002",
    "proxmox003",
    "proxmox004",
    "proxmox005",
    "proxmox006",
    "proxmox007",
    "PC08",
    "PC09",
    "PC10",
    "PC11",
    "PC12",
    "PC13",
    "PC14",
    "PC15",
    "PC16",
    "PC17",
    "PC18",
    "PC19",
    "PC20",
    "PC21"
};

// パルス幅（ミリ秒） - 電源ボタンを押す時間
#define POWER_PULSE_MS 500

// 長押し時間（ミリ秒） - 強制シャットダウン用
#define POWER_LONG_PRESS_MS 5000
