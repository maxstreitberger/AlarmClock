# Alarm Clock / Timer ⏰
## Description
This project is about building an alarm clock. The alarm clock is powered by an Arduino Uno microcontroller. It is able to present the current time via an RTC and a four-digit seven-segment display. The amount of time someone wants to sleep is controlled via two buttons, and the alarm gets armed via an additional button. LEDs indicate in which state the alarm clock is. After the alarm is armed, the WS2821 LED ring shows the progress of how much time has already been passed.

## Components
- Arduino Uno (Microcontroller)
- RTC (Real Time Clock)
- 4 Digit 7 Segment Display
- WS2812 LED ring (24 LEDs)
- Passive Buzzer
- 4x LEDs as Indicators (Red, Green, Blue, Yellow)
- 2x Shift Registers (74hc595)
- Aluminum Foil
- 4x Buttons
- 4x PN2222 transistors
- 2x 220 Ohm resistor
- 2x 680 ohm resistor
- 4x 4,7k Ohm resistor
- 6x 10k Ohm resistor

## Prerequisites
- Arduino IDE
- FastLED by Daniel Garcia
- CapacitiveSensor by Paul Bagder, Paul Stoffregen
- DS3232RTC by Jack Christensen
- Pitches
