# Lucky Resistor's Boldport 3x7 Driver

This is a very simple driver for the Boldport project #28 "3x7". It is specifically written for Arduino UNO and compatible boards which use the *ATmega328P* chip and running at *16 MHz*.

## How to Connect the Project

Two pin configurations are supported.
Basically sequentially connect the pins from the Arduino board, starting with pin 2 or pin 4 to the display. Except the GND, which is not in sequence.

| 3x7 Pin      | Arduino (default) | Arduino (alternative) |
| ------------ | ----------------- | --------------------- |
| GND          | GND               | GND                   |
| Digit 1 Sink | Pin 2             | Pin 4                 |
| Digit 2 Sink | Pin 3             | Pin 5                 |
| Digit 3 Sink | Pin 4             | Pin 6                 |
| Segment g    | Pin 5             | Pin 7                 |
| Segment f    | Pin 6             | Pin 8                 |
| Segment e    | Pin 7             | Pin 9                 |
| Segment d    | Pin 8             | Pin 10                |
| Segment c    | Pin 9             | Pin 11                |
| Segment b    | Pin 10            | Pin 12                |
| Segment a    | Pin 11            | Pin 13                |

The operative configuration is specified with the `Pins` parameter to the library `initialize` function.

There is no functional difference in the configurations, they just leave different pins free.
For example, the alternative configuration allows a program to use external interrupts on pins 2 and 3.

**Please note:** This driver shall just provide a well designed example how to drive this kind of displays. Most likely you will have to adapt the driver for your particular use case.

## License

(c)2018 by Lucky Resistor. See LICENSE for details.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
