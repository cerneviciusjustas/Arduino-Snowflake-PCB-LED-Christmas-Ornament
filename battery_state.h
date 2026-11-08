#pragma once

typedef enum {
  STATE_OFF = 0,
  STATE_CHARGING_SWITCH_OFF,
  STATE_CHARGING_SWITCH_ON,
  STATE_SWITCH_ON_ONLY
} PowerState;

void powerStateUpdate(void);
PowerState powerStateGet(void);

// callbacks (weak by default)
void onChargingSwitchOff(void);
void onChargingSwitchOn(void);
void onSwitchOnOnly(void);
void onPowerOff(void);

float readBatteryVoltage();
static bool debounceBool(bool rawValue, bool *stableValue, unsigned long *lastChange);