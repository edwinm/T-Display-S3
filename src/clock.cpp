#include <TFT_eSPI.h>
#include <OneButton.h>
#include <SPIFFS.h>
#include <ezTime.h>
#include "WiFi.h"
#include "pins.h"
#include "user_data.h"

// Documentation
// TFT_eSPI       https://github.com/Bodmer/TFT_eSPI#readme
// OneButton      https://github.com/mathertel/OneButton#readme
// ezTime         https://github.com/ropg/ezTime

enum button_status {
  ON,
  OFF
};

// Constants
#define BRIGHTNESS_PIN 38
#define BRIGHTNESS_CHANNEL 0
#define BRIGHTNESS_VALUE 160
#define WIDTH 320
#define HEIGHT 170

// Status
#define STATUS_W 250
#define STATUS_H 30

// Clock
#define CLOCK_HEIGHT 105

// Colors
#define BACKGROUND TFT_BLACK
#define STATUS_FG TFT_WHITE
#define STATUS_BG 0x104a // Very dark green. See https://rgbcolorpicker.com/565
#define CLOCK_YELLOW 0xfd00
#define CLOCK_GREY TFT_LIGHTGREY


// Display objects
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite statusSprite = TFT_eSprite(&tft);
TFT_eSprite clockSprite = TFT_eSprite(&tft);

// Buttons
button_status button1_status = OFF;
button_status button2_status = OFF;

// Buttons
OneButton button1(PIN_BUTTON_1, true);
OneButton button2(PIN_BUTTON_2, true);

Timezone myTZ;

const char* status = NULL;

String timeShown = "";

bool is24 = true;
bool isYellow = true;

void render() {
  if (status) {
    // Clear the screen
    tft.fillScreen(BACKGROUND);

    // Clear the sprite
    statusSprite.fillSprite(STATUS_BG);
  
      // Set text properties
    statusSprite.setTextColor(STATUS_FG, STATUS_BG);
    statusSprite.setTextDatum(MC_DATUM); // Middle center alignment
    
    // Draw text - parameters: text, x, y, font
    statusSprite.drawString(status, STATUS_W/2, STATUS_H/2, 2);

    // Push to display
    statusSprite.pushSprite((WIDTH-STATUS_W) / 2, (HEIGHT-STATUS_H) / 2);
  } else {
    clockSprite.fillSprite(BACKGROUND);
    clockSprite.setTextColor(isYellow ? CLOCK_YELLOW : CLOCK_GREY, BACKGROUND);
    int textWidth = clockSprite.textWidth(timeShown);
    clockSprite.drawString(timeShown, 320 - textWidth, 0);
    clockSprite.pushSprite(0, 45);
  }
  
  tft.fillRect(WIDTH-8, HEIGHT-40, 8, 40, button1_status == ON ? TFT_LIGHTGREY : BACKGROUND);
  tft.fillRect(WIDTH-8, 0, 8, 40, button2_status == ON ? TFT_LIGHTGREY : BACKGROUND);
}

void renderTime() {
  const char* format = is24 ? "H:i" : "h:i";
  const String currentTime = myTZ.dateTime(format);
  if (currentTime != timeShown) {
    timeShown = currentTime;
    render();
  }
}

uint32_t getMilliVolt() {
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
  statusSprite.createSprite(STATUS_W, STATUS_H);
  clockSprite.createSprite(WIDTH, CLOCK_HEIGHT);
  
  // Set display brightness
  ledcSetup(BRIGHTNESS_CHANNEL, 10000, 8);
  ledcAttachPin(BRIGHTNESS_PIN, BRIGHTNESS_CHANNEL);
  ledcWrite(BRIGHTNESS_CHANNEL, BRIGHTNESS_VALUE);
  
  // Draw the static content
  setStatus("Starting up");

  button1.attachPress([]() {
    button1_status = ON;
    render();
  });

  button1.attachClick([]() {
    button1_status = OFF;
    is24 = !is24;
    render();
  });

  button2.attachPress([]() {
    button2_status = ON;
    render();
  });

  button2.attachClick([]() {
    button2_status = OFF;
    isYellow = !isYellow;
    render();
  });

  // Connect to Wifi.
  char buffer[100]; // SSID max length is 32 + status text length
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
    }
    delay(5000);
  }

  Serial.println(WiFi.localIP());

  if (!myTZ.setCache(0)) {
    myTZ.setLocation(TIMEZONE);
  }

  if (!SPIFFS.begin()) {
    setStatus("Flash memory initialisation failed");
    while (1) yield();
  }

  if (!SPIFFS.exists("/BungeeRegular100.vlw")) {
    setStatus("Font missing in flash memory");
    while (1) yield();
  }

  clockSprite.loadFont("BungeeRegular100");

  setStatus(NULL);
}

void loop() {
  button1.tick();
  button2.tick();
  events();
  renderTime();
  delay(20); // Add a delay to reduce power consumption
}