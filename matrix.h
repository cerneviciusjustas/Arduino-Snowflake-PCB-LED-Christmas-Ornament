#pragma once
#include <stdint.h>

// ===== GPIO masks (must match your existing defines) =====
extern const uint8_t COL_PC_MASK;
extern const uint8_t COL_PD_MASK;
extern const uint8_t ROW_PC_MASK;
extern const uint8_t ROW_PB_MASK;

// ===== Matrix primitives =====
void setRow(uint8_t row);
void randomColumns(uint8_t density);
void matrixInit();
void matrixResetGPIO();
