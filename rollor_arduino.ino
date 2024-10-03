#include <HTTPClient.h>
#include <WiFi.h>

#include <FastLED.h>

#include <ArduinoJson.h>

#ifndef WIFI_SSID
#define WIFI_SSID "KANASSI-PC 8866"  // WiFi SSID (2.4GHz only)
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "w2puwmf3"  // WiFiパスワード
#endif

String url = "http://192.168.137.1:8000";

#define LED_PIN     D8
#define NUM_LEDS   60
#define BRIGHTNESS  50
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    delay(500);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
    WiFiClient client;
    HTTPClient http;

    if (!http.begin(client, url)) {
        Serial.println("Failed HTTPClient begin!");
        return;
    }

    Serial.println("HTTPClient begin!");
    http.addHeader("Content-Type", "application/json");
    int responseCode = http.GET();
    String body = http.getString();
    Serial.println(responseCode);
    Serial.println(body);

    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    JsonObject color = doc["color"];
    int color_red = color["red"]; // 255
    int color_green = color["green"]; // 0
    int color_blue = color["blue"]; // 255

    Serial.println("red:" + String(color_red) + "green:" + String(color_green) + "blue:" + String(color_blue));

    http.end();

    for(int i=0; i<NUM_LEDS; i++) { // For each pixel...
      leds[i] = CRGB( color_red, color_green, color_blue);
      FastLED.show();
      delay(1); // Pause before next pass through loop
    }

    delay(500);
}