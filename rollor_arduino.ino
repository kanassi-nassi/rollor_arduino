#include <WiFi.h>
#include <HTTPClient.h>
#include <FastLED.h>
#include <ArduinoJson.h>
#include <WebServer.h>

// WiFiの認証情報
#ifndef WIFI_SSID
#define WIFI_SSID "kamashi-pc"  // WiFiのSSID（ネットワーク名）
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "kamashipc"  // WiFiのパスワード
#endif

// 固定IPアドレスの設定
IPAddress local_IP(10, 42, 0, 184);    // ESPに固定するIPアドレス（10.42.0.xネットワーク内）
IPAddress gateway(10, 42, 0, 1);       // ゲートウェイアドレス（通常はルーターのアドレス）
IPAddress subnet(255, 255, 255, 0);    // サブネットマスク（ネットワーク範囲の定義）
IPAddress primaryDNS(8, 8, 8, 8);      // プライマリDNSサーバー（Google DNSを使用）
IPAddress secondaryDNS(8, 8, 4, 4);    // セカンダリDNSサーバー（予備のDNS、こちらもGoogle DNS）

// LEDストリップの設定
#define LED_PIN     D8              // LEDストリップが接続されるピン
#define NUM_LEDS    60              // LEDの個数
#define BRIGHTNESS  50              // LEDの明るさ（0〜255の範囲）
#define LED_TYPE    WS2812B         // 使用しているLEDのタイプ
#define COLOR_ORDER GRB              // LEDストリップの色順（Green, Red, Blue）
CRGB leds[NUM_LEDS];                // LEDのデータを格納する配列

// Webサーバーオブジェクトを作成（ポート5000で動作）
WebServer server(5000);              // ESPがホストするサーバーをポート5000で起動

// POSTリクエストを処理してLEDの色を変更する関数
void handleColorPost() {
  // リクエストがボディを持っているか確認
  if (server.hasArg("plain")) {
    String body = server.arg("plain");  // リクエストのボディを取得

    // 受信したJSONデータを解析
    StaticJsonDocument<200> doc;  // JSON解析用のドキュメントを作成
    DeserializationError error = deserializeJson(doc, body);  // JSONをデシリアライズ

    // JSONの解析に失敗した場合、エラーメッセージを返す
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      server.send(400, "text/plain", "Invalid JSON");  // 400エラーを返す
      return;
    }

    // JSONドキュメントからRGB値を取得
    int red = doc["r"];
    int green = doc["g"];
    int blue = doc["b"];

    // 取得したRGB値を使って全てのLEDの色を変更
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(red, green, blue);
    }
    FastLED.show();  // LEDストリップを更新

    // クライアントに成功メッセージを返す
    server.send(200, "text/plain", "Color updated successfully");

    // 受信したRGB値をシリアルモニターに表示
    Serial.printf("Received color: R=%d, G=%d, B=%d\n", red, green, blue);
  } else {
    // ボディが存在しない場合、エラーメッセージを返す
    server.send(400, "text/plain", "No JSON body received");
  }
}

void setup() {
  Serial.begin(115200);  // デバッグ用のシリアル通信を開始

  // WiFiに接続する前に固定IPアドレスを設定
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Failed to configure static IP");
  }

  // WiFiに接続開始
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);  // 1秒待って再試行
    Serial.println("WiFiに接続中...");
  }
  Serial.println("WiFiに接続完了");

  // LEDストリップを初期化
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);  // LEDストリップの明るさを設定

  // POSTリクエストのエンドポイント（/ws）を定義し、ハンドラ関数を設定
  server.on("/ws", HTTP_POST, handleColorPost);

  // サーバーを開始
  server.begin();
  Serial.println("HTTPサーバーが起動しました");
}

void loop() {
  server.handleClient();  // クライアントからのリクエストを処理
}
