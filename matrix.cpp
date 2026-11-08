#include "matrix.h"
#include <Arduino.h>

const uint8_t COL_PC_MASK = (1<<PC3)|(1<<PC4)|(1<<PC5);
const uint8_t COL_PD_MASK = (1<<PD4)|(1<<PD5)|(1<<PD6);
const uint8_t ROW_PC_MASK = (1<<PC2)|(1<<PC1);
const uint8_t ROW_PB_MASK = (1<<PB5)|(1<<PB4)|(1<<PB3)|(1<<PB2)|(1<<PB1)|(1<<PB0);

/* ===================== MATRIX ===================== */
void randomColumns(uint8_t density) {
  uint8_t pc = 0, pd = 0;

  if (random(256) < density) pc |= (1<<PC3);
  if (random(256) < density) pc |= (1<<PC4);
  if (random(256) < density) pc |= (1<<PC5);
  if (random(256) < density) pd |= (1<<PD4);
  if (random(256) < density) pd |= (1<<PD5);
  if (random(256) < density) pd |= (1<<PD6);

  PORTC = (PORTC & ~COL_PC_MASK) | pc;
  PORTD = (PORTD & ~COL_PD_MASK) | pd;
}

void setRow(uint8_t row) {
  PORTC &= ~ROW_PC_MASK;
  PORTB &= ~ROW_PB_MASK;

  if (row == 0) PORTC |= (1<<PC2);
  else if (row == 1) PORTC |= (1<<PC1);
  else PORTB |= (1 << (5 - (row - 2)));
}

void matrixInit() {
  DDRC |= ROW_PC_MASK | COL_PC_MASK;
  DDRB |= ROW_PB_MASK;
  DDRD |= COL_PD_MASK;

  PORTC &= ~(ROW_PC_MASK | COL_PC_MASK);
  PORTB &= ~ROW_PB_MASK;
  PORTD &= ~COL_PD_MASK;
}

void matrixResetGPIO(){
  // Set Port C pins (COL and ROW) to 0
PORTC &= ~(COL_PC_MASK | ROW_PC_MASK);

// Set Port D pins to 0
PORTD &= ~(COL_PD_MASK);

// Set Port B pins to 0
PORTB &= ~(ROW_PB_MASK);
}