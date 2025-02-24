#include <TFT_eSPI.h>

// Constants
#define BRIGHTNESS_PIN 38
#define BRIGHTNESS_CHANNEL 0
#define BRIGHTNESS_VALUE 160

// Display objects
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

// Colors
const uint16_t BACKGROUND = TFT_BLACK;

void drawStaticContent() {
  // Clear the sprite
  sprite.fillSprite(BACKGROUND);
  
  // Draw rectangle - parameters: x, y, width, height, color
  sprite.drawRect(40, 60, 240, 80, TFT_RED);
  
  // Set text properties
  sprite.setTextColor(TFT_GREEN, BACKGROUND);
  sprite.setTextDatum(MC_DATUM); // Middle center alignment
  
  // Draw text - parameters: text, x, y, font
  sprite.drawString("Hello world!", 160, 100, 4);
  
  // Push to display
  sprite.pushSprite(0, 0);
}

void setup() {
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
  drawStaticContent();
}



void loop() {
  // Nothing to do in the loop since it's static content
  delay(1000); // Add a delay to reduce power consumption
}