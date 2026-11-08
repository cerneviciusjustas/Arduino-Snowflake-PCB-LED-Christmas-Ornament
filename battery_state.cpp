#include "battery_state.h"
#include <Arduino.h>

static PowerState currentState = STATE_CHARGING_SWITCH_OFF;

#define CHARGING_IND 7   // PD7
#define STANDBY_IND 2    // PD2
#define BATT_MEASURE A0  // PC0
#define DEBOUNCE_MS 200
#define BATTERY_SAMPLE_MS 500


// ------------------------------------------------

PowerState powerStateGet(void) {
  return currentState;
}

// ------------------------------------------------
static bool debounceBool(bool rawValue, bool *stableValue, unsigned long *lastChange) {
  unsigned long now = millis();

  if (rawValue != *stableValue) {
    if (now - *lastChange >= DEBOUNCE_MS) {
      *stableValue = rawValue;
    }
  } else {
    *lastChange = now;
  }

  return *stableValue;
}



float readBatteryVoltage() {
  const float ADC_REF = 1.1;
  const float DIVIDER = 4.0;
  uint16_t raw = analogRead(BATT_MEASURE);
  return (raw * ADC_REF / 1023.0) * DIVIDER;
}

static bool isCablePlugged(void) {
  static bool initialized = false;
  static bool stableCable;
  static unsigned long lastChange;

  bool rawCable =
    (!digitalRead(CHARGING_IND) || !digitalRead(STANDBY_IND));

  if (!initialized) {
    stableCable = rawCable;
    lastChange = millis();
    initialized = true;
  }

  return debounceBool(rawCable, &stableCable, &lastChange);
}


static bool isSwitchOn(void) {
  static bool initialized = false;
  static float cachedVoltage;
  static unsigned long lastSample;

  unsigned long now = millis();

  if (!initialized) {
    cachedVoltage = readBatteryVoltage();
    lastSample = now;
    initialized = true;
  } else if (now - lastSample >= BATTERY_SAMPLE_MS) {
    cachedVoltage = readBatteryVoltage();
    lastSample = now;
  }

  return (cachedVoltage >= 2.0f);
}




// ------------------------------------------------

void powerStateUpdate(void) {
  PowerState newState;

  bool cable = isCablePlugged();
  bool switchOn = isSwitchOn();

  if (cable && !switchOn) {
    newState = STATE_CHARGING_SWITCH_OFF;
  } else if (cable && switchOn) {
    newState = STATE_CHARGING_SWITCH_ON;
  } else if (!cable && switchOn) {
    newState = STATE_SWITCH_ON_ONLY;
  } else {
    newState = STATE_OFF;
  }

  static bool firstRun = true;

  if (!firstRun && newState == currentState)
    return;

  firstRun = false;


  currentState = newState;

  switch (currentState) {
    case STATE_CHARGING_SWITCH_OFF:
      onChargingSwitchOff();
      break;

    case STATE_CHARGING_SWITCH_ON:
      onChargingSwitchOn();
      break;

    case STATE_SWITCH_ON_ONLY:
      onSwitchOnOnly();
      break;

    case STATE_OFF:
    default:
      onPowerOff();
      break;
  }
}


__attribute__((weak)) void onChargingSwitchOff(void) {}
__attribute__((weak)) void onChargingSwitchOn(void) {}
__attribute__((weak)) void onSwitchOnOnly(void) {}
__attribute__((weak)) void onPowerOff(void) {}
