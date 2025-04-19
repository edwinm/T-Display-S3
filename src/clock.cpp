#include <TFT_eSPI.h>
#include <OneButton.h>
#include <FS.h>
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

// Colors
#define BACKGROUND TFT_BLACK
#define STATUS_FG TFT_WHITE
#define STATUS_BG 0x104a // Very dark green. See https://rgbcolorpicker.com/565

// Display objects
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

// Buttons
button_status button1_status = OFF;
button_status button2_status = OFF;

// Buttons
OneButton button1(PIN_BUTTON_1, true);
OneButton button2(PIN_BUTTON_2, true);

Timezone myTZ;

const char* status = NULL;

String timeShown = "";

void render() {
  // Clear the screen
  tft.fillScreen(BACKGROUND);

  // Clear the sprite
  sprite.fillSprite(STATUS_BG);
  
  if (status) {
      // Set text properties
    sprite.setTextColor(STATUS_FG, STATUS_BG);
    sprite.setTextDatum(MC_DATUM); // Middle center alignment
    
    // Draw text - parameters: text, x, y, font
    sprite.drawString(status, STATUS_W/2, STATUS_H/2, 2);
  } else {
    
    sprite.drawString(timeShown, STATUS_W/2, STATUS_H/2, 2);
  }
  
  // Push to display
  sprite.pushSprite((WIDTH-STATUS_W) / 2, (HEIGHT-STATUS_H) / 2);

  if (button1_status == ON) {
    tft.fillRect(WIDTH-8, HEIGHT-40, 8, 40, TFT_LIGHTGREY);
  }

  if (button2_status == ON) {
    tft.fillRect(WIDTH-8, 0, 8, 40, TFT_LIGHTGREY);
  }
}

void renderTime() {
  const String currentTime = myTZ.dateTime("H:i");
  if (currentTime != timeShown) {
    timeShown = currentTime;
    render();
  }
}

uint32_t getVolt() {
  return (analogRead(PIN_BAT_VOLT) * 2 * 3.3 * 1000) / 4096;
}

void setStatus(const char* message) {
  status = message;
  render();
}

void printLocalTime()
{
  Serial.println(myTZ.dateTime("H:i"));
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
    button1_status = ON;
    render();
  });

  button1.attachClick([]() {
    // const uint16_t volt = getVolt();
    // Serial.print(volt);
    // Serial.println(" millivolt");
    button1_status = OFF;
    render();
  });

  button2.attachPress([]() {
    button2_status = ON;
    render();
  });

  button2.attachClick([]() {
    button2_status = OFF;
    render();
    printLocalTime();
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
    }
    delay(5000);
  }

  Serial.println(WiFi.localIP());

  if (!myTZ.setCache(0)) {
    myTZ.setLocation(TIMEZONE);
  }

  if (!SPIFFS.begin()) {
    setStatus("SPIFFS initialisation failed!");
    while (1) yield();
  }

  if (!SPIFFS.exists("/BungeeRegular28.vlw")) {
    setStatus("Font missing in SPIFFS!");
    while (1) yield();
  }

  sprite.loadFont("BungeeRegular28");

  setStatus(NULL);



}

void loop() {
  button1.tick();
  button2.tick();
  events();
  renderTime();
  delay(20); // Add a delay to reduce power consumption
}