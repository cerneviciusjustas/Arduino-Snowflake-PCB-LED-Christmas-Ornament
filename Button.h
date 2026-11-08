#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

void buttonInit();
void buttonUpdate();

void onShortPress();
void onLongPress();
void onVeryLongPress();
void onDoubleClick();

#endif
