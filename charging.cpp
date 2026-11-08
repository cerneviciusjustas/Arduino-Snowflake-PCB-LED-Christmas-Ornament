#include "charging.h"
#include "matrix.h"
#include <Arduino.h>

// ================= CONFIG =================

#define CHARGE_STEP_TIME_US 250000UL  // time per row
#define CHARGE_ROW_ON_US 1000UL       // scan speed
#define CHARGE_DENSITY 15

// =========================================

enum ChargeState {
  CHARGE_OFF,
  CHARGE_FILL_UP,
  CHARGE_FILL_DOWN
};

static ChargeState state = CHARGE_OFF;
static uint32_t stateTime = 0;

// render state
static uint8_t currentRow = 0;
static uint8_t filledRows = 0;
static uint32_t lastRowTime = 0;

// ------------------------------------------------

void chargingStart(void) {
  state = CHARGE_FILL_UP;
  stateTime = micros();
  filledRows = 0;
  currentRow = 0;
}

void chargingStop(void) {
  state = CHARGE_OFF;
}

bool chargingIsActive(void) {
  return state != CHARGE_OFF;
}

// ------------------------------------------------

void doCharging(void) {
  if (state == CHARGE_OFF) {
    PORTC &= ~COL_PC_MASK;
    PORTD &= ~COL_PD_MASK;
    return;
  }

  uint32_t now = micros();

  // advance fill level
  if (now - stateTime >= CHARGE_STEP_TIME_US) {
    stateTime = now;

    if (state == CHARGE_FILL_UP) {
      if (filledRows < 8) {
        filledRows++;
      } else {
        state = CHARGE_FILL_DOWN;
      }
    } else if (state == CHARGE_FILL_DOWN) {
      if (filledRows > 0) {
        filledRows--;
      } else {
        state = CHARGE_OFF;
        return;
      }
    }
  }

  // row scanning
  if (now - lastRowTime < CHARGE_ROW_ON_US)
    return;

  lastRowTime = now;

  PORTC &= ~COL_PC_MASK;
  PORTD &= ~COL_PD_MASK;

  setRow(currentRow);

  // draw filled rows only
  if (
    (state == CHARGE_FILL_UP && currentRow < filledRows) || (state == CHARGE_FILL_DOWN && currentRow < filledRows)) {
    randomColumns(CHARGE_DENSITY);
  }

  currentRow = (currentRow + 1) & 7;
}
