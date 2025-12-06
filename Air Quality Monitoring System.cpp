// Air Quality Monitoring System - ESP32 + MQ2 + OLED + ThingSpeak + Telegram
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi Credentials
const char* ssid = "Wokwi-GUEST";
const char* password = " ";

// ThingSpeak API
const char* thingspeak_api_key = " ";
const char* thingspeak_url = "";

// Telegram Bot
const char* bot_token = "";
const char* chat_id = "";

// MQ2 Sensor Pin
#define MQ2_PIN 34

// OLED Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(1000);
        Serial.print(".");
        attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
    } else {
        Serial.println("\nFailed to Connect. Check WiFi Credentials or Router!");
    }

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println("OLED initialization failed!");
        while (true);
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10, 20);
    display.println("Starting...");
    display.display();
    delay(2000);
}

void loop() {
    int sensorValue = analogRead(MQ2_PIN);
    float sensorVoltage = sensorValue * (3.3 / 4095.0);
    float air_quality = (sensorVoltage * 16); // Approximate PPM estimation
    Serial.print("Air Quality (PPM): ");
    Serial.println(air_quality);

    // Determine Air Quality Status
    String airQualityStatus;
    if (air_quality < 50) {
        airQualityStatus = "Good";
    } else if (air_quality < 100) {
        airQualityStatus = "Moderate";
    } else if (air_quality < 150) {
        airQualityStatus = "Unhealthy for Sensitive Groups";
    } else if (air_quality < 200) {
        airQualityStatus = "Unhealthy";
    } else if (air_quality < 300) {
        airQualityStatus = "Very Unhealthy";
    } else {
        airQualityStatus = "Hazardous";
    }

    // Display on OLED
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print("AQI: ");
    display.println(air_quality);
    display.setTextSize(1);
    display.setCursor(0, 40);
    display.println(airQualityStatus);
    display.display();

    // Send data to ThingSpeak
    if (WiFi.status() == WL_CONNECTED) {
        String url = String(thingspeak_url) + thingspeak_api_key + "&field1=" + String(air_quality);
        HTTPClient http;
        http.begin(url);
        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
            Serial.println("Data sent to ThingSpeak");
        } else {
            Serial.println("Failed to send data");
        }
        http.end();
    }

    // Send Telegram Alert if AQI is too high
    if (air_quality > 100) {
        sendTelegramAlert(air_quality, airQualityStatus);
    }
    
    delay(15000); // Wait 15 seconds
}

void sendTelegramAlert(float aqi, String status) {
    String message = "Air Quality Alert! AQI: " + String(aqi) + " (" + status + ")";
    String url = "https://api.telegram.org/bot" + String(bot_token) + "/sendMessage?chat_id=" + chat_id + "&text=" + message;
    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
        Serial.println("Alert sent to Telegram");
    } else {
        Serial.println("Failed to send Telegram alert");
    }
    http.end();
}


