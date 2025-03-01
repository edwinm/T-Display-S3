#include <TFT_eSPI.h>
#include <OneButton.h>
#include "WiFi.h"
#include "pins.h"
#include "wifi-credentials.h"

// Documentation
// TFT_eSPI       https://github.com/Bodmer/TFT_eSPI#readme
// OneButton      https://github.com/mathertel/OneButton#readme

// Constants
#define BRIGHTNESS_PIN 38
#define BRIGHTNESS_CHANNEL 0
#define BRIGHTNESS_VALUE 160

// Status
#define STATUS_W 250
#define STATUS_H 30

// Colors
#define BACKGROUND TFT_BLACK
#define STATUS_FG TFT_WHITE
#define STATUS_BG TFT_DARKGREEN


// Display objects
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

// Buttons
OneButton button1(PIN_BUTTON_1, true);
OneButton button2(PIN_BUTTON_2, true);

const char* status = NULL;

void render() {
  // Clear the sprite
  sprite.fillSprite(STATUS_BG);
  
  if (status) {
      // Set text properties
    sprite.setTextColor(STATUS_FG, STATUS_BG);
    sprite.setTextDatum(MC_DATUM); // Middle center alignment
    
    // Draw text - parameters: text, x, y, font
    sprite.drawString(status, STATUS_W/2, STATUS_H/2, 2);
  }
  
  // Push to display
  sprite.pushSprite((320-STATUS_W) / 2, (170-STATUS_H) / 2);
}

uint32_t getVolt() {
  return (analogRead(PIN_BAT_VOLT) * 2 * 3.3 * 1000) / 4096;
}

void setStatus(const char* message) {
  status = message;
  render();
}

void setup() {
  Serial.begin(9600);

  // Initialize display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(BACKGROUND);
  sprite.createSprite(STATUS_W, STATUS_H);
  
  // Set display brightness
  ledcSetup(BRIGHTNESS_CHANNEL, 10000, 8);
  ledcAttachPin(BRIGHTNESS_PIN, BRIGHTNESS_CHANNEL);
  ledcWrite(BRIGHTNESS_CHANNEL, BRIGHTNESS_VALUE);
  
  // Draw the static content
  setStatus("Starting up");

  // Giving it a little time because the serial monitor doesn't
  // immediately attach. Want the firmware that's running to
  // appear on each upload.
  delay(1000);

  button1.attachPress([]() {
    Serial.println("Button 1 press");
    render();
  });

  button1.attachClick([]() {
    Serial.println("Button 1 click");
    const uint16_t volt = getVolt();
    Serial.print(volt);
    Serial.println(" millivolt");
    render();
  });

  button2.attachPress([]() {
    Serial.println("Button 2 press");
    render();
  });

  button2.attachClick([]() {
    Serial.println("Button 2 click");
    render();
  });

  // Connect to Wifi.
  char buffer[100]; // SSID max length is 32
  sprintf(buffer, "Connecting to %s", WIFI_SSID);
  setStatus(buffer);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    if (WiFi.status() == WL_CONNECT_FAILED) {
      setStatus("Failed to connect to WiFi");
      return;
    }
    delay(5000);
  }

  setStatus("Connected");
  // Serial.println(WiFi.localIP());
}

void loop() {
  button1.tick();
  button2.tick();
  delay(10); // Add a delay to reduce power consumption
}