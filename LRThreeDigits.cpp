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
// on the ATmega328P chip. The lower byte of the value maps to the bits
// of port D and the upper byte to port B. 


/// The number of digits of the display.
///
const uint8_t cDigitCount = 3;

/// The bits on the ports to drive the digit sinks.
///
const uint16_t cDigitPortBit[cDigitCount] = {
    0b0000000000000100, // d1
    0b0000000000001000, // d2
    0b0000000000010000, // d3
};

/// The number of segments for each digit.
///
const uint8_t cSegmentCount = 7;

/// The bits on the ports to drive each segment.
///
/// The segments of the display are enumerated in 
/// reverse as shown in the illustration below:
///
/// ```
/// .-3-.
/// 4   2
/// :-0-:
/// 5   1
/// .-6-.
/// ```
///
const uint16_t cSegmentPortBit[cSegmentCount] = {
    0b0000000000100000, // g
    0b0000000001000000, // f
    0b0000000010000000, // e
    0b0000000100000000, // d
    0b0000001000000000, // c
    0b0000010000000000, // b
    0b0000100000000000, // a
};

/// The mask for all segment bits for the ports.
///
const uint16_t cSegmentMask = 0b0000111111100000;

/// The mask for all used bits on the ports.
///
const uint16_t cOutputMask = 0b0000111111111100;


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
    {'0',  0b01111110},
    {'1',  0b00000110},
    {'2',  0b01101101},
    {'3',  0b01001111},
    {'4',  0b00010111},
    {'5',  0b01011011},
    {'6',  0b01111011},
    {'7',  0b00001110},
    {'8',  0b01111111},
    {'9',  0b01011111},
    {'a',  0b00111111},
    {'b',  0b01110011},
    {'c',  0b01100001},
    {'d',  0b01100111},
    {'e',  0b01111001},
    {'f',  0b00111001},
    {'_',  0b01000000},
    {'*',  0b00011101}, // degree symbol
    {'\'', 0b00010000},
    {'"',  0b00010100},
    {' ',  0b00000000},
    {'\0', 0b00000000}
};


// Variables
// ---------------------------------------------------------------------------


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
/// This `constexpr` will allow the compiler to resolve this function call
/// into the optimized code.
///
/// @param mask The combined mask.
/// @return The mask for port D.
///
constexpr uint8_t getPortDMask(const uint16_t mask)
{
    return static_cast<uint8_t>(mask & 0x00ffu);
}

/// Get the port B mask part from the combined mask.
///
/// @param mask The combined mask.
/// @return The mask for port B.
///
constexpr uint8_t getPortBMask(const uint16_t mask)
{
    return static_cast<uint8_t>(mask >> 8);
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
    portD &= ~getPortDMask(cOutputMask);
    portB &= ~getPortBMask(cOutputMask);
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
/// @param digitMask The abstract mask for the digits to display.
/// @return The port mask for the pin bits to enable.
///
uint16_t getPortBitsFromDigitMask(uint8_t digitMask)
{
    uint16_t portBits = 0;
    for (uint8_t i = 0; i < cSegmentCount; ++i) {
        if ((digitMask & 0b1) != 0) {
            portBits |= cSegmentPortBit[i];
        }
        digitMask >>= 1;
    }
    return portBits;
}


// Interface Implementation
// ---------------------------------------------------------------------------


void initialize(Frequency frequency)
{
    // Initialize the ports. Set all used pins to output and into low state.
    DDRD |= getPortDMask(cOutputMask);
    DDRB |= getPortBMask(cOutputMask);
    PORTD &= ~getPortDMask(cOutputMask);
    PORTB &= ~getPortBMask(cOutputMask);
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


void setDigits(const char *text)
{
    // Check if we got a pointer to text.
    if (text == nullptr) {
        return;
    }
    // Calculate the new bit masks for the ports.
    uint16_t portBits[cDigitCount];
    for (uint8_t i = 0; i < cDigitCount; ++i) {
        portBits[i] = cDigitPortBit[i];
    }
    for (uint8_t i = 0; i < cDigitCount; ++i) {
        if (text[i] == '\0') {
            break;
        }
        portBits[i] |= getPortBitsFromDigitMask(getDigitMask(text[i]));
    }
    // Update the output masks for the display.
    cli();
    for (uint8_t i = 0; i < cDigitCount; ++i) {
        gDigitOutputMask[i] = portBits[i];
    }
    sei();
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


