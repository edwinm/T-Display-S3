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

// Display objects
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

// Buttons
OneButton button1(PIN_BUTTON_1, true);
OneButton button2(PIN_BUTTON_2, true);

// Colors
const uint16_t BACKGROUND = TFT_BLACK;

uint32_t rectColor = TFT_RED;
uint32_t textColor = TFT_GREEN;

void render() {
  // Clear the sprite
  sprite.fillSprite(BACKGROUND);
  
  // Draw rectangle - parameters: x, y, width, height, color
  sprite.drawRect(40, 60, 240, 80, rectColor);
  
  // Set text properties
  sprite.setTextColor(textColor, BACKGROUND);
  sprite.setTextDatum(MC_DATUM); // Middle center alignment
  
  // Draw text - parameters: text, x, y, font
  sprite.drawString("Hello world!", 160, 100, 4);
  
  // Push to display
  sprite.pushSprite(0, 0);
}

uint32_t getVolt() {
  return (analogRead(PIN_BAT_VOLT) * 2 * 3.3 * 1000) / 4096;
}

void setup() {
  Serial.begin(9600);
  // Giving it a little time because the serial monitor doesn't
  // immediately attach. Want the firmware that's running to
  // appear on each upload.
  delay(2000);

  // Initialize display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(BACKGROUND);
  sprite.createSprite(320, 170);
  
  // Set display brightness
  ledcSetup(BRIGHTNESS_CHANNEL, 10000, 8);
  ledcAttachPin(BRIGHTNESS_PIN, BRIGHTNESS_CHANNEL);
  ledcWrite(BRIGHTNESS_CHANNEL, BRIGHTNESS_VALUE);
  
  // Draw the static content
  render();

  button1.attachPress([]() {
    Serial.println("Button 1 press");
    rectColor = TFT_BLUE;
    render();
  });

  button1.attachClick([]() {
    Serial.println("Button 1 click");
    const uint16_t volt = getVolt();
    Serial.print(volt);
    Serial.println(" millivolt");

    rectColor = TFT_RED;
    render();
  });

  button2.attachPress([]() {
    Serial.println("Button 2 press");
    textColor = TFT_BLUE;
    render();
  });

  button2.attachClick([]() {
    Serial.println("Button 2 click");
    textColor = TFT_GREEN;
    render();
  });

  // Connect to Wifi.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Connecting...");

  while (WiFi.status() != WL_CONNECTED) {
    // Check to see if connecting failed.
    // This is due to incorrect credentials
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Failed to connect to WIFI. Please verify credentials: ");
      Serial.println();
      Serial.print("SSID: ");
      Serial.println(WIFI_SSID);
      Serial.print("Password: ");
      Serial.println(WIFI_PASS);
      Serial.println();
    }
    delay(5000);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Hello World, I'm connected to the internets!!");
}

void loop() {
  button1.tick();
  button2.tick();
  delay(10); // Add a delay to reduce power consumption
}