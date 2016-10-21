/*
 * DS1307RTC.h - library for DS1307 RTC
  
  Copyright (c) Michael Margolis 2009
  This library is intended to be uses with Arduino Time library functions

  The library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  30 Dec 2009 - Initial release
  5 Sep 2011 updated for Arduino 1.0
 */


#include "DS1307RTC.h"
#include <assert.h>

static time_t g_time_offset = 0;

DS1307RTC::DS1307RTC()
{
}
  
// PUBLIC FUNCTIONS
time_t DS1307RTC::get()   // Aquire data from buffer and convert to time_t
{
  tmElements_t tm;
  if (read(tm) == false) return 0;
  return(makeTime(tm));
}

bool DS1307RTC::set(time_t t)
{
  tmElements_t tm;
  breakTime(t, tm);
  return write(tm); 
}

extern "C" time_t time(time_t *t);

// Aquire data from the RTC chip in BCD format
bool DS1307RTC::read(tmElements_t &tm)
{
    time_t t = time(NULL) + g_time_offset;
    breakTime(t, tm);
    return true;
}

bool DS1307RTC::write(tmElements_t &tm)
{
    time_t t = makeTime(tm);

    g_time_offset = t - time(NULL);
    return true;
}

unsigned char DS1307RTC::isRunning()
{
    return true;
}

void DS1307RTC::setCalibration(char calValue)
{
    assert(0);
}

char DS1307RTC::getCalibration()
{
    assert(0);
    return 0;
}

// PRIVATE FUNCTIONS

// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t DS1307RTC::dec2bcd(uint8_t num)
{
  return ((num/10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t DS1307RTC::bcd2dec(uint8_t num)
{
  return ((num/16 * 10) + (num % 16));
}

bool DS1307RTC::exists = true;

DS1307RTC RTC = DS1307RTC(); // create an instance for the user

