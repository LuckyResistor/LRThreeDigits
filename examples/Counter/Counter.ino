
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


#include <LRThreeDigits.h>


void setup() {
  // Initialize the driver.
  lr::ThreeDigits::initialize();
}


void loop() {
  // Count from 0 to 999 and repeat.
  char buffer[10];
  for (uint16_t i = 0; i < 1000; ++i) {
    sprintf(buffer, "%3d", i);
    lr::ThreeDigits::setDigits(buffer);
    delay(100);
  }
}


