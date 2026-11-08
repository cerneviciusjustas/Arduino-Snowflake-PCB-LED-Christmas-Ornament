# Arduino-Snowflake-PCB-LED-Christmas-Ornament
Arduino-compatible LED snowflake PCB ornament with programmable Christmas light effects.
https://www.tindie.com/products/justtech/arduino-snowflake-pcb-led-christmas-ornament/

## What is it?

A unique, rechargeable LED Christmas snowflake ornament powered by an ATmega328PB microcontroller. It features 64 LEDs: 48 white LEDs and 16 individually addressable WS2812 RGB LEDs, creating beautiful festive lighting effects.

The snowflake has a built-in 300 mAh Li-ion battery and USB-C connectivity for both charging and programming.

## Why did you make it?

I wanted to create something different from ordinary Christmas decorations – a festive ornament that combines beautiful lighting with the creativity of programmable electronics.

It's designed for people who enjoy Christmas, DIY electronics, and Arduino-compatible projects.

## What makes it special?

- **64 LEDs:** 48 bright white LEDs and 16 individually addressable WS2812 RGB LEDs.
- **Rechargeable battery:** Built-in 300 mAh Li-ion battery.
- **USB-C connectivity:** Convenient charging and programming through a single USB connection.
- **Programmable:** Based on the ATmega328PB microcontroller and compatible with the Arduino development ecosystem.
- **Custom lighting effects:** Create your own animations and festive patterns using the addressable RGB LEDs.
- **5V boost converter:** Provides a 5V supply for the electronics and LEDs.
- **Battery monitoring:** Built-in battery voltage measurement.
- **Battery protection:** Integrated battery protection circuit.
- **Easy controls:** One multifunction push button and a physical ON/OFF slide switch.
- **USB-to-UART converter:** Allows direct microcontroller programming over USB.

---

# User Instructions

## 1. Button Controls

The snowflake has one multifunction button. Different press types activate different functions.

| Button action | Function |
|---|---|
| Short press | Change LED brightness |
| Double-click (two clicks within 300 ms) | Change RGB LED color mode |
| Long press (longer than 1 second) | Change animation mode |
| Very long press (longer than 3 seconds) | Toggle animation display while charging with the ON/OFF switch turned OFF |

## 2. Brightness Control

Short-press the button to cycle through the brightness levels.

Each press increases brightness. When maximum brightness is reached, the next press returns to the lowest brightness level.

There are 8 brightness levels, ranging from 31 to 255.

The brightness setting is saved automatically and restored after restarting the snowflake.

## 3. Color Selection

Double-click the button to cycle through the available color modes for the 16 RGB LEDs.

Available colors and effects:

| Color mode | Description |
|---|---|
| Cool | Cool blue-toned light |
| Red | Red lighting |
| Green | Green lighting |
| Blue | Blue lighting |
| Purple | Purple lighting |
| Cyan | Cyan lighting |
| Yellow | Yellow lighting |
| Orange | Orange lighting |
| Pink | Pink lighting |
| Random | Individual RGB LEDs display changing random colors with slowly shifting hues |

The selected color mode is saved automatically.

## 4. Animation Modes

Long-press the button to cycle through five animation modes:

| Mode | Description |
|---|---|
| **1. Combined** | White glitter LEDs and colorful RGB animations work together. |
| **2. Glitter Only** | The 48 white LEDs blink randomly, creating a sparkling snow effect. |
| **3. Colorful Sparkle & Twinkle** | The 16 RGB LEDs produce colorful sparkling bursts and softly fading twinkles. |
| **4. Random Color Twinkle** | The 16 RGB LEDs randomly illuminate in different colors and gradually fade out. |
| **5. Random Twinkle + Glitter** | Random-color RGB twinkles run together with the 48 white glitter LEDs. |

Each long press advances to the next animation mode. After the fifth mode, the sequence starts again from the first.

The selected animation mode is saved automatically.

## 5. Charging and Power Control

- **ON/OFF slide switch:** Turns the snowflake's normal operating mode on or off.
- **USB charging:** Connect the snowflake to a USB power source to recharge its built-in battery.
- **Charging animation:** When the ON/OFF switch is turned OFF and a USB cable is connected, the snowflake automatically displays a special charging animation.
- **Animations while charging:** When the snowflake is switched OFF and connected to USB power, press and hold the button for more than 3 seconds to enable or disable the normal LED animations. This allows you to enjoy the snowflake's lighting effects while charging.

## 6. Settings Memory

Your selected brightness, color mode, and animation mode are automatically saved in non-volatile memory.

When you turn the snowflake off and switch it on again, your previously selected settings are restored.
