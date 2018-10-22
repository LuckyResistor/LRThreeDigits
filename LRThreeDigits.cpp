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
#include "LRThreeDigits.h"


#include <Arduino.h>

#include <avr/interrupt.h>


namespace lr {
namespace ThreeDigits {


// Configuration
// ---------------------------------------------------------------------------


// Note:
// The following 16 bit masks are the combined bits for GPIO ports B+D
// on the ATmega328P chip.
//
// The masks and variables are all 0-based, with significant bits
// starting from bit 0.
//
// Data is only shifted into correct bit position at the point
// of updating the port register, after which the lower byte
// will be written to port D and the upper byte to port B.
//


/// The mask for all bits used.
///
const uint16_t cPortBitMask = 0b0000001111111111;

/// The number of digits of the display.
///
const uint8_t cDigitCount = 3;

/// The bits on the ports to drive the digit sinks.
///
const uint16_t cDigitPortBit[cDigitCount] = {
    0b0000000000000100, // d3
    0b0000000000000010, // d2
    0b0000000000000001, // d1
};

/// The number of segments for each digit.
///
const uint8_t cSegmentCount = 7;

/// The bits on the ports to drive each segment.
///
/// ```
/// .-0-.  .-a-.
/// 5   1  f   b
/// :-6-:  :-g-:
/// 4   2  e   c
/// .-3-.  .-d-.
/// ```
///
const uint16_t cSegmentPortBit[cSegmentCount] = {
    0b0000001000000000, // a
    0b0000000100000000, // b
    0b0000000010000000, // c
    0b0000000001000000, // d
    0b0000000000100000, // e
    0b0000000000010000, // f
    0b0000000000001000, // g
};

/// Bit flip array to rotate a digit.
///
const uint8_t cSegmentRotateMap[cSegmentCount] = {
    3, 4, 5, 0, 1, 2, 6
};

/// Structure to define the mask for one digit.
///
/// The mask is a combination of bits which are mapped to the indexes
/// of the segments as defined in `cSegmentPortBit`. Therefore this definitions
/// are independent from the bit masks above.
///
struct DigitMask {
    char c; ///< The character to map.
    uint8_t mask; ///< The mask for this character.
};

/// A list with known characters to display.
///
/// Extend this list as you like, but make sure it always
/// ends with a `\0` zero character entry.
///
const DigitMask cDigitMask[] = {
    {'0',  0x3f},
    {'1',  0x06},
    {'2',  0x5b},
    {'3',  0x4f},
    {'4',  0x66},
    {'5',  0x6d},
    {'6',  0x7d},
    {'7',  0x07},
    {'8',  0x7f},
    {'9',  0x6f},
    {'a',  0x77},
    {'b',  0x7c},
    {'c',  0x39},
    {'d',  0x5e},
    {'e',  0x79},
    {'f',  0x71},
    {'_',  0x08},
    {'*',  0x63}, // degree symbol
    {'\'', 0x20},
    {'"',  0x0a},
    {' ',  0x00},
    {'\0', 0x00}
};


// Variables
// ---------------------------------------------------------------------------

/// The number of lower unused pins in PORTD.
///
uint8_t gPortBitOffset = 2;

/// The orientation of the display.
///
Orientation gOrientation = Orientation::ConnectorOnTop;

/// The current lighten digit index.
///
uint8_t gCurrentDigitIndex = 0;

/// The pre-calculated register masks for all digits.
///
volatile uint16_t gDigitOutputMask[cDigitCount] = {
    cDigitPortBit[0],
    cDigitPortBit[1],
    cDigitPortBit[2]
};


// Helper Functions
// ---------------------------------------------------------------------------


/// Get the port D mask part from the combined mask.
///
/// @param mask The combined mask.
/// @return The mask for port D.
///
uint8_t getPortDMask(const uint16_t mask)
{
    return static_cast<uint8_t>(mask << gPortBitOffset & 0x00ffu);
}

/// Get the port B mask part from the combined mask.
///
/// @param mask The combined mask.
/// @return The mask for port B.
///
uint8_t getPortBMask(const uint16_t mask)
{
    return static_cast<uint8_t>(mask >> (8 - gPortBitOffset));
}

/// Update the ports to light up the next digit.
///
/// This function is called from the interrupt to light up the next digit
/// on the display.
///
void updatePorts()
{
    uint8_t portD = PORTD;
    uint8_t portB = PORTB;
    portD &= ~getPortDMask(cPortBitMask);
    portB &= ~getPortBMask(cPortBitMask);
    portD |= getPortDMask(gDigitOutputMask[gCurrentDigitIndex]);
    portB |= getPortBMask(gDigitOutputMask[gCurrentDigitIndex]);
    PORTD = portD;
    PORTB = portB;
    ++gCurrentDigitIndex;
    if (gCurrentDigitIndex == cDigitCount) {
        gCurrentDigitIndex = 0;
    }
}

/// Search the mask for a character in the table.
///
/// @param c The character to search.
/// @return The mask for the character.
///
uint8_t getDigitMask(char c)
{
    const DigitMask *current = cDigitMask;
    while (current->c != c) {
        ++current;
        if (current->c == '\0') {
            break;
        }
    }
    return current->mask;
}

/// Convert the digit mask into the actual port mask.
///
/// @param segmentMask The abstract mask for the digits to display.
/// @return The port mask for the pin bits to enable.
///
uint16_t getPortBitsFromSegmentMask(uint8_t segmentMask)
{
    uint16_t portBits = 0;
    for (uint8_t i = 0; i < cSegmentCount; ++i) {
        if ((segmentMask & 0b1) != 0) {
            uint8_t targetSegmentIndex;
            if (gOrientation == Orientation::ConnectorOnTop) {
                targetSegmentIndex = i;
            } else {
                targetSegmentIndex = cSegmentRotateMap[i];
            }
            portBits |= cSegmentPortBit[targetSegmentIndex];
        }
        segmentMask >>= 1;
    }
    return portBits;
}

/// Update the port bits.
///
/// @param portBits Array with the port bits to display.
///
void updatePortBits(uint16_t *portBits)
{
    cli();
    if (gOrientation == Orientation::ConnectorOnTop) {
        for (uint8_t i = 0; i < cDigitCount; ++i) {
            gDigitOutputMask[i] = portBits[i];
            gDigitOutputMask[i] |= cDigitPortBit[i];
        }
    } else {
        for (uint8_t i = 0; i < cDigitCount; ++i) {
            const uint8_t sourceIndex = cDigitCount-i-1;
            gDigitOutputMask[i] = portBits[sourceIndex];
            gDigitOutputMask[i] |= cDigitPortBit[i];
        }
    }
    sei();
}


// Interface Implementation
// ---------------------------------------------------------------------------

void initialize(
    Frequency frequency,
    Orientation orientation,
    Pins pins)
{
    // Store the orientation.
    gOrientation = orientation;
    // Resolve and save the pin offset.
    if(pins == Pins::From2to11) gPortBitOffset = 2;
    else gPortBitOffset = 4;

    // Initialize the ports. Set all used pins to output and into low state.
    DDRD |= getPortDMask(cPortBitMask);
    DDRB |= getPortBMask(cPortBitMask);
    PORTD &= ~getPortDMask(cPortBitMask);
    PORTB &= ~getPortBMask(cPortBitMask);
    // Initialize timer2 for the display refresh.
    ASSR = 0; // Synchronous internal clock.
    if (frequency != Frequency::Insane) {
        TCCR2A = 0; // Normal operation.
        // Simple use the lower 3 bits for pre-scaling to set the
        // multiplexer speed.
        TCCR2B = static_cast<uint8_t>(frequency);
        OCR2A = 0; // Ignore the compare
        OCR2B = 0; // Ignore the compare
        TIMSK2 = _BV(TOIE2); // Interrupt on overflow.
    } else {
        TCCR2A = _BV(WGM21); // CTC mode.
        TCCR2B = _BV(CS20); // No multiplexing.
        OCR2A = 0x80; // Only count to 0x80
        TIMSK2 = _BV(OCIE2A); // Interrupt on overflow.
    }
    sei(); // Allow interrupts.
}


void setOrientation(Orientation orientation)
{
    gOrientation = orientation;
}


void setDigits(const char *text)
{
    // Check if we got a pointer to text.
    if (text == nullptr) {
        return;
    }
    // Calculate the new bit masks for the ports.
    uint16_t portBits[cDigitCount];
    for (uint8_t i = 0; i < cDigitCount; ++i) {
        portBits[i] = 0;
    }
    for (uint8_t i = 0; i < cDigitCount; ++i) {
        if (text[i] == '\0') {
            break;
        }
        const uint8_t segmentMask = getDigitMask(text[i]);
        portBits[i] = getPortBitsFromSegmentMask(segmentMask);
    }
    // Update the output masks for the display.
    updatePortBits(portBits);
}


void setSegments(const uint8_t *segmentMasks)
{
    uint16_t portBits[cDigitCount];
    for (uint8_t i = 0; i < cDigitCount; ++i) {
        const uint8_t segmentMask = segmentMasks[i];
        portBits[i] = getPortBitsFromSegmentMask(segmentMask);
    }
    // Update the output masks for the display.
    updatePortBits(portBits);
}


}
}


/// The interrupt handler for timer 2.
///
ISR(TIMER2_OVF_vect)
{
    lr::ThreeDigits::updatePorts();
}
ISR(TIMER2_COMPA_vect)
{
    lr::ThreeDigits::updatePorts();
}


