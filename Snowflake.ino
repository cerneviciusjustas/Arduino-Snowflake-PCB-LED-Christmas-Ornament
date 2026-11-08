#include <Adafruit_NeoPixel.h>
#include "Button.h"
#include "charging.h"
#include "matrix.h"
#include "battery_state.h"
#include <EEPROM.h>


/* ===================== POWER & BATTERY ===================== */
#define BATT_MEASURE A0  // PC0
#define CHARGING_IND 7   // PD7
#define STANDBY_IND 2    // PD2


/* ===================== LED MATRIX ===================== */
// Rows
#define ROW_PC_MASK ((1 << PC1) | (1 << PC2))
#define ROW_PB_MASK ((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4) | (1 << PB5))

// Columns
#define COL_PC_MASK ((1 << PC3) | (1 << PC4) | (1 << PC5))
#define COL_PD_MASK ((1 << PD4) | (1 << PD5) | (1 << PD6))

#define ROW_ON_MIN_US 50   // minimum row ON time
#define ROW_ON_MAX_US 800  // maximum row ON time
#define BASE_DENSITY 3
#define BURST_DENSITY 10
#define BURST_CHANCE 100
#define BURST_DURATION 50000

/* ===================== NEOPIXEL STRIP ===================== */
#define LED_PIN 3
#define NUM_LEDS 16

#define BRIGHTNESS 64
#define BRIGHTNESS_STEP 32
#define FRAMES_PER_SECOND 30
#define COOLING 2
#define TWINKLING 5
#define FLICKER 1

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
uint8_t heat[NUM_LEDS];
uint16_t ledHue[NUM_LEDS];     // stores assigned hue per LED
int8_t ledHueDrift[NUM_LEDS];  // signed drift direction & speed



enum ColorMode {
  COLOR_COOL,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_PURPLE,
  COLOR_CYAN,
  COLOR_YELLOW,
  COLOR_ORANGE,
  COLOR_PINK,
  COLOR_RANDOM_LED,
  COLOR_COUNT
};

/* ================== EEPROM ======================== */

#define EEPROM_MAGIC 0xA5
#define EEPROM_START 0
#define EEPROM_RECORDS 32  // 32 saves before wrap-around

struct Settings {
  uint8_t magic;
  uint16_t sequence;
  uint8_t colorMode;
  uint8_t animationMode;
  uint8_t brightness;
};

Settings currentSettings;
uint16_t currentSequence = 0;
uint8_t currentSlot = 0;



/* ===================== TIMING ===================== */
unsigned long nextBeat = 0;
unsigned long nextTwinkle = 300;
unsigned long loops = 0;
bool beatStarted = false;
int beatInterval = 15000;

/* ======================MISC==========================*/

bool forceShowAnimation = false;

enum AnimationMode {
  MODE_BOTH,
  MODE_GLITTER_ONLY,
  MODE_SPARKLETWINKLE_ONLY,
  MODE_THEOTHERTWINKLE,
  MODE_THEOTHERTWINKLE_SPARKLE,
  MODE_COUNT
};

// for the other twinkle
float redStates[NUM_LEDS];
float blueStates[NUM_LEDS];
float greenStates[NUM_LEDS];
float Fade = 0.99;
unsigned long lastTwinkleUpdate = 0;
const uint16_t twinkleInterval = 5;  // Equivalent to delay(6)

/* ===================== SETUP ===================== */
void setup() {
  loadSettings();
  // button on PE0
  buttonInit();

  // NeoPixels
  strip.begin();
  strip.setBrightness(currentSettings.brightness);
  strip.show();

  // Power & battery
  setupADC_1V1();
  pinMode(CHARGING_IND, INPUT);
  pinMode(STANDBY_IND, INPUT);

  // Matrix
  matrixInit();

  randomSeed(analogRead(BATT_MEASURE));

  // will be shown if realy plugged in
  chargingStart();
}

/* ===================== BATTERY ===================== */
void setupADC_1V1() {
  analogReference(INTERNAL);
  delay(1);
}



// ================= CALLBACKS ===============
void onShortPress() {
  if (powerStateGet() != STATE_CHARGING_SWITCH_OFF || forceShowAnimation) {
    onBrightnessButton();
  }
}

void onLongPress() {
  if (powerStateGet() != STATE_CHARGING_SWITCH_OFF || forceShowAnimation) {
    onAnimationModeButton();
    if (currentSettings.animationMode == MODE_GLITTER_ONLY) {
      strip.clear();
      strip.show();
    }

    if (currentSettings.animationMode == MODE_SPARKLETWINKLE_ONLY) {
      matrixResetGPIO();
    }
  }
}

void onVeryLongPress() {
  if (powerStateGet() == STATE_CHARGING_SWITCH_OFF) {
    forceShowAnimation = !forceShowAnimation;
    strip.clear();
    strip.show();
    if (currentSettings.animationMode == MODE_SPARKLETWINKLE_ONLY || currentSettings.animationMode == MODE_BOTH) {
      resetSparkleTwinkleState();
    }
  }
}

void onDoubleClick() {
  if (powerStateGet() != STATE_CHARGING_SWITCH_OFF || forceShowAnimation) {
    onColorModeButton();
  }
}

void onChargingSwitchOff(void) {
  strip.clear();
  strip.show();
}

void onChargingSwitchOn(void) {
  forceShowAnimation = false;
}
// -----------------



void doGlitter() {
  static uint8_t row = 0;
  static uint32_t lastTime = 0;
  static uint32_t rowOnTime = 0;
  static bool inBurst = false;
  static uint32_t burstEnd = 0;

  uint32_t now = micros();

  if (now - lastTime >= rowOnTime) {
    lastTime = now;
    rowOnTime = random(ROW_ON_MIN_US, ROW_ON_MAX_US);

    PORTC &= ~COL_PC_MASK;
    PORTD &= ~COL_PD_MASK;

    row = (row + 1) & 7;
    setRow(row);

    if (!inBurst && random(12000) < BURST_CHANCE) {
      inBurst = true;
      burstEnd = now + BURST_DURATION;
    }
    if (inBurst && (int32_t)(now - burstEnd) >= 0) {
      inBurst = false;
    }

    uint8_t baseDensity = inBurst ? BURST_DENSITY : BASE_DENSITY;
    baseDensity += random(-1, 2);
    if (baseDensity > 255) baseDensity = 255;

    randomColumns(baseDensity);
  }
}

/* ===================== TWINKLE ===================== */
uint32_t TwinkleColor(uint8_t t, uint8_t i) {
  uint8_t r = 0, g = 0, b = 0;

  switch (currentSettings.colorMode) {
    case COLOR_COOL:
      r = (t * 10) / 255;
      g = (t * 80) / 255;
      b = t;
      break;

    case COLOR_RED:
      r = t;
      break;

    case COLOR_GREEN:
      g = t;
      break;

    case COLOR_BLUE:
      b = t;
      break;

    case COLOR_PURPLE:
      r = t;
      b = (t * 150) / 255;
      break;

    case COLOR_CYAN:
      g = t;
      b = t;
      break;

    case COLOR_YELLOW:
      r = t;
      g = t;
      break;

    case COLOR_ORANGE:
      r = t;
      g = (t * 120) / 255;
      break;

    case COLOR_PINK:
      r = t;
      b = (t * 80) / 255;
      break;

    case COLOR_RANDOM_LED:
      return strip.gamma32(
        strip.ColorHSV(
          ledHue[i],  // drifting hue
          255,
          t));
  }

  return strip.Color(r, g, b);
}

void Twinkle() {
  uint8_t seeds = random(2, NUM_LEDS / 2);

  for (uint8_t i = 0; i < NUM_LEDS; i++)
    heat[i] = (heat[i] > COOLING) ? heat[i] - COOLING : 0;

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    if (heat[i] > 0 && currentSettings.colorMode == COLOR_RANDOM_LED) {
      ledHue[i] += ledHueDrift[i];  // slow hue drift
    }
  }

  for (uint8_t j = 0; j < seeds; j++) {
    if (random(256) < TWINKLING) {
      uint8_t idx = random(NUM_LEDS);
      heat[idx] = random(40, 200);

      if (currentSettings.colorMode == COLOR_RANDOM_LED) {
        ledHue[idx] = random(65535);
        ledHueDrift[idx] = random(-3, 4);  // slow drift speed
        if (ledHueDrift[idx] == 0) ledHueDrift[idx] = 1;
      }
    }
  }

  for (uint8_t k = 0; k < NUM_LEDS; k++)
    if (heat[k] && random(256) < FLICKER)
      heat[k] = (heat[k] > 250) ? 255 : heat[k] + 5;

  for (uint8_t i = 0; i < NUM_LEDS; i++)
    strip.setPixelColor(i, TwinkleColor(heat[i], i));

  nextTwinkle += 1000 / FRAMES_PER_SECOND;
}

void Sparkle() {
  uint8_t seeds = random(NUM_LEDS / 2, NUM_LEDS);
  for (uint8_t i = 0; i < seeds; i++) {
    uint8_t idx = random(NUM_LEDS);
    heat[idx] = random(180, 255);

    if (currentSettings.colorMode == COLOR_RANDOM_LED) {
      ledHue[idx] = random(65535);
      ledHueDrift[idx] = random(-4, 5);
      if (ledHueDrift[idx] == 0) ledHueDrift[idx] = 1;
    }
  }

  nextBeat += beatInterval;
  loops++;
}

void resetSparkleTwinkleState() {
  beatStarted = false;
  nextBeat = millis();
  nextTwinkle = millis();

  // Optional but STRONGLY recommended:
  memset(heat, 0, sizeof(heat));
}


void doSparkleTwinkle() {
  if (!beatStarted) {
    beatStarted = true;
    nextBeat = millis();
    Sparkle();
  } else if ((long)(millis() - nextBeat) > 0) {
    Sparkle();
  }

  if ((long)(millis() - nextTwinkle) > 0)
    Twinkle();

  strip.show();
}


void theOtherTwinkle() {
  // Check if it's time to update
  if (millis() - lastTwinkleUpdate < twinkleInterval) {
    return;  // Exit early if 6ms hasn't passed yet
  }
  lastTwinkleUpdate = millis();  // Record the current time for the next cycle

  // --- Start of your original logic ---
  if (random(25) == 1) {
    uint16_t i = random(NUM_LEDS);
    if (redStates[i] < 1 && greenStates[i] < 1 && blueStates[i] < 1) {
      redStates[i] = random(256);
      greenStates[i] = random(256);
      blueStates[i] = random(256);
    }
  }

  for (uint16_t l = 0; l < NUM_LEDS; l++) {
    if (redStates[l] > 1 || greenStates[l] > 1 || blueStates[l] > 1) {
      strip.setPixelColor(l, redStates[l], greenStates[l], blueStates[l]);

      // Fading logic
      redStates[l] = (redStates[l] > 1) ? redStates[l] * Fade : 0;
      greenStates[l] = (greenStates[l] > 1) ? greenStates[l] * Fade : 0;
      blueStates[l] = (blueStates[l] > 1) ? blueStates[l] * Fade : 0;
    } else {
      strip.setPixelColor(l, 0, 0, 0);
    }
  }
  strip.show();
}


int recordAddress(uint8_t slot) {
  return EEPROM_START + slot * sizeof(Settings);
}

void saveSettings() {
  currentSequence++;
  currentSlot = (currentSlot + 1) % EEPROM_RECORDS;

  currentSettings.magic = EEPROM_MAGIC;
  currentSettings.sequence = currentSequence;

  EEPROM.put(recordAddress(currentSlot), currentSettings);
}

void onColorModeButton() {
  currentSettings.colorMode =
    (currentSettings.colorMode + 1) % COLOR_COUNT;

  saveSettings();
}

void onAnimationModeButton() {
  currentSettings.animationMode =
    (currentSettings.animationMode + 1) % MODE_COUNT;

  if (currentSettings.animationMode == MODE_SPARKLETWINKLE_ONLY || currentSettings.animationMode == MODE_BOTH) {
    resetSparkleTwinkleState();
  }

  saveSettings();
}


void onBrightnessButton() {
  if (currentSettings.brightness == 255) {
    // Roll over only after reaching max
    currentSettings.brightness = BRIGHTNESS_STEP - 1;
  } else {
    uint8_t next = currentSettings.brightness + BRIGHTNESS_STEP;

    // Clamp to 255 if we overshoot
    if (next < currentSettings.brightness || next > 255) {
      currentSettings.brightness = 255;
    } else {
      currentSettings.brightness = next;
    }
  }

  strip.setBrightness(currentSettings.brightness);
  saveSettings();
}



void loadSettings() {
  Settings temp;
  uint16_t bestSeq = 0;
  bool found = false;

  for (uint8_t i = 0; i < EEPROM_RECORDS; i++) {
    EEPROM.get(recordAddress(i), temp);

    if (temp.magic == EEPROM_MAGIC && temp.colorMode < COLOR_COUNT && temp.animationMode < MODE_COUNT && temp.brightness <= 255) {

      if (!found || temp.sequence > bestSeq) {
        bestSeq = temp.sequence;
        currentSlot = i;
        currentSettings = temp;
        found = true;
      }
    }
  }

  if (found) {
    currentSequence = bestSeq;
  } else {
    // First boot ever
    currentSettings.magic = EEPROM_MAGIC;
    currentSettings.sequence = 0;
    currentSettings.colorMode = COLOR_CYAN;
    currentSettings.animationMode = MODE_BOTH;
    currentSettings.brightness = BRIGHTNESS_STEP - 1;

    currentSlot = 0;
    currentSequence = 0;

    // Write defaults ONCE
    EEPROM.put(recordAddress(currentSlot), currentSettings);
  }
}



/* ===================== LOOP ===================== */
void loop() {

  buttonUpdate();  // MUST be called continuously
  powerStateUpdate();

  if (powerStateGet() == STATE_CHARGING_SWITCH_OFF && !forceShowAnimation) {
    doCharging();
  } else {

    // to prevent charging animation on start.
    if (chargingIsActive()) {
      chargingStop();
    }

    switch (currentSettings.animationMode) {

      case MODE_GLITTER_ONLY:
        doGlitter();  // white LEDs only
        break;

      case MODE_SPARKLETWINKLE_ONLY:
        doSparkleTwinkle();  // color LEDs only
        break;

      case MODE_BOTH:
        doGlitter();         // white LEDs
        doSparkleTwinkle();  // color LEDs
        break;

      case MODE_THEOTHERTWINKLE:
        theOtherTwinkle();
        break;

      case MODE_THEOTHERTWINKLE_SPARKLE:
        theOtherTwinkle();
        doGlitter();
        break;
    }
  }
}
