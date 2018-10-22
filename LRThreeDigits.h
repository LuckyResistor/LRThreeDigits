#pragma once
//
// Lucky Resistor's Boldport 3x7 Driver
// ---------------------------------------------------------------------------
// (c)2018 by Lucky Resistor. See LICENSE for details.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//


#include <stdint.h>


/// @namespace lr::ThreeDigits
///
/// This driver is written for Boldport project #28 "3x7".
/// It is specifically written for Arduino UNO and compatible boards
/// which use the ATmega328P chip.
///
/// | Arduino Pin  | Project Pin       |
/// | ------------ | ----------------- |
/// | GND          | GND               |
/// | Pin 2        | Digit 1 Sink      |
/// | Pin 3        | Digit 2 Sink      |
/// | Pin 4        | Digit 3 Sink      |
/// | Pin 5        | Segment g         |
/// | Pin 6        | Segment f         |
/// | Pin 7        | Segment e         |
/// | Pin 8        | Segment d         |
/// | Pin 9        | Segment c         |
/// | Pin 10       | Segment b         |
/// | Pin 11       | Segment a         |
///
/// Basically sequentially connect the pins from the Arduino board,
/// starting with Pin 2 to the display. Except the GND, which is
/// not in sequence.
///
/// Please note: This driver shall just provide a well designed example
/// how to drive this kind of display. Most likely you will have to
/// adapt the driver for your particular use case.
///


namespace lr {
namespace ThreeDigits {


/// The multiplexing frequency for the digits.
///
/// Faster frequencies will reduce the visible flickering of the
/// display, but will use more overall CPU time.
///
/// The written frequency is calculated using the ATmega328P
/// running at 16 MHz. The frequency is for a whole display with.
/// 3 digits. Each digit is lighten up equally in this time.
/// In other words, e.g. the first digit is displayed every 1/f seconds.
///
enum class Frequency : uint8_t {
    UselessSlow = 0x7, /// ~20 Hz - Visible annoying flickering.
    VerySlow = 0x6, /// ~81 Hz - Flickering invisible for most people.
    Slow = 0x5, /// ~163 Hz - Flickering not visible for humans.
    Normal = 0x4, /// ~325 Hz - Good reasonable compromise.
    Fast = 0x3, /// ~650 Hz - Fast refresh with almost no inference.
    Faster = 0x2, /// ~2.6 kHz - Very fast
    VeryFast = 0x1, /// ~20 kHz - A perfect display quality.
    Insane = 0x0, /// ~40 kHz - If your MCU is dedicated for just this display.
};

/// The orientation of the display.
///
enum class Orientation : uint8_t {
    ConnectorOnTop, ///< The connector of the display is at the top side.
    ConnectorOnBottom ///< The connector of the display is at the bottom side.
};

/// The pin connection options.
///
enum class Pins : uint8_t {
    From2to11, ///< connector plugged into pins 2-11 (default)
    From4to13  ///< connector plugged into pins 4-13
};

/// Initialize the driver.
///
/// This will initialise the driver and start the interrupt to
/// refresh the display. Initially, the display is blank, until
/// you call `setDigits` with some text.
///
/// @param frequency The multiplexer frequency.
///
void initialize(
    Frequency frequency = Frequency::Normal,
    Orientation orientation = Orientation::ConnectorOnTop,
    Pins pins = Pins::From2to11);

/// Set the orientation of the display.
///
/// Usually you just keep the orientation setting from the
/// `initialize` method. You can change it later using this
/// method. Changing the orientation will not affect the
/// currently displayed content. To actually flip the content
/// you have to call `setDigits()` or `setSegments()`.
///
void setOrientation(Orientation orientation);

/// Set the text for the digits.
///
/// Pass a string with one or more characters to this function
/// to display them. A string shorter than the number of
/// digits on the display, the text is displayed left aligned
/// on the display.
///
void setDigits(const char *text);

/// Set the segments manually for a digit.
///
/// The segments and the corresponding bits are shown in the
/// illustration below:
///
/// ```
/// .-0-.  .-a-.
/// 5   1  f   b
/// :-6-:  :-g-:
/// 4   2  e   c
/// .-3-.  .-d-.
/// ```
///
/// @param segmentMasks The segment masks for all digits
///   of the display from left to right.
///
void setSegments(const uint8_t *segmentMasks);


}
}
