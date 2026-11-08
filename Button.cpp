#include "Button.h"
#include <avr/io.h>
#include <avr/interrupt.h>

// ================= CONFIG =================
#define BUTTON_BIT              0       // PE0
#define DEBOUNCE_TIME           30UL
#define LONG_PRESS_TIME         1000UL
#define VERY_LONG_PRESS_TIME    3000UL
#define DOUBLE_CLICK_TIME       300UL

// ================= ISR STATE ==============
volatile bool irqEvent = false;
volatile bool irqLevel = false;

// ================= LOGIC STATE ============
static bool buttonPressed = false;
static bool longReported = false;
static bool veryLongReported = false;

static unsigned long pressStartTime = 0;
static unsigned long lastIRQTime = 0;

// ================= INIT ===================
void buttonInit()
{
    DDRE  &= ~(1 << BUTTON_BIT);   // input
    PORTE |=  (1 << BUTTON_BIT);   // pull-up

    // ===== CORRECT PCINT CONFIG =====
    PCICR  |= (1 << PCIE3);        // Enable PORTE PCINT
    PCMSK3 |= (1 << PCINT24);      // Enable PCINT24 (PE0)

    sei();
}

// ================= ISR ====================
ISR(PCINT3_vect)
{
    irqLevel = !(PINE & (1 << BUTTON_BIT)); // active LOW
    irqEvent = true;
}

// ================= UPDATE =================
void buttonUpdate()
{
    unsigned long now = millis();

    static bool lastStableLevel = false;
    static bool pending = false;
    static unsigned long debounceStart = 0;

    static bool veryLongFired = false;

    static uint8_t clickCount = 0;
    static unsigned long firstClickTime = 0;

    // --------- IRQ received ---------
    if (irqEvent) {
        irqEvent = false;
        pending = true;
        debounceStart = now;
    }

    // --------- Debounce & state confirm ---------
    if (pending && (now - debounceStart) >= DEBOUNCE_TIME) {
        pending = false;

        bool level = irqLevel;

        // ----- Press confirmed -----
        if (level && !lastStableLevel) {
            lastStableLevel = true;
            buttonPressed = true;
            pressStartTime = now;
            veryLongFired = false;
        }

        // ----- Release confirmed -----
        else if (!level && lastStableLevel) {
            lastStableLevel = false;
            buttonPressed = false;

            unsigned long duration = now - pressStartTime;

            // ----- Very long already handled -----
            if (veryLongFired) {
                clickCount = 0;
                return;
            }

            // ----- Long press -----
            if (duration >= LONG_PRESS_TIME) {
                clickCount = 0;
                onLongPress();
                return;
            }

            // ----- Short press (candidate for double click) -----
            clickCount++;

            if (clickCount == 1) {
                firstClickTime = now;
            }
            else if (clickCount == 2) {
                if ((now - firstClickTime) <= DOUBLE_CLICK_TIME) {
                    clickCount = 0;
                    onDoubleClick();
                } else {
                    // Too slow → treat as new first click
                    clickCount = 1;
                    firstClickTime = now;
                }
            }
        }
    }

    // --------- Very long press while holding ---------
    if (buttonPressed && !veryLongFired) {
        if ((now - pressStartTime) >= VERY_LONG_PRESS_TIME) {
            veryLongFired = true;
            clickCount = 0;
            onVeryLongPress();
        }
    }

    // --------- Single click timeout ---------
    if (clickCount == 1 && (now - firstClickTime) > DOUBLE_CLICK_TIME) {
        clickCount = 0;
        onShortPress();
    }
}

