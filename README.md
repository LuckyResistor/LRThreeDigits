# Lucky Resistor's Boldport 3x7 Driver

This is a very simple driver for the Boldport project #28 "3x7". It is specifically written for Arduino UNO and compatible boards which use the *ATmega328P* chip and running at *16 MHz*.

## How to Connect the Project
| Arduino Pin  | Project Pin       |
| ------------ | ----------------- |
| GND          | GND               |
| Pin 2        | Digit 1 Sink      |
| Pin 3        | Digit 2 Sink      |
| Pin 4        | Digit 3 Sink      |
| Pin 5        | Segment g         |
| Pin 6        | Segment f         |
| Pin 7        | Segment e         |
| Pin 8        | Segment d         |
| Pin 9        | Segment c         |
| Pin 10       | Segment b         |
| Pin 11       | Segment a         |

Basically sequentially connect the pins from the Arduino board, starting with pin 2 to the display. Except the GND, which is not in sequence.

**Please note:** This driver shall just provide a well designed example how to drive this kind of displays. Most likely you will have to adapt the driver for your particular use case.

## License

(c)2018 by Lucky Resistor. See LICENSE for details.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
