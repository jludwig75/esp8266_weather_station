/* 
  Ticker.cpp - esp8266 library that calls functions periodically

  Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.
 
  This library is free software; you can redistribute it and/or
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
*/

#include <stddef.h>
#include <stdint.h>
#include <thread>
#include <chrono>

#include "Ticker.h"

using namespace std;

Ticker::Ticker()
: _timer(0),
_milliseconds(0),
_repeat(false),
_callback(NULL),
_arg(0),
_stop_timer_thread(false)
{
}

Ticker::~Ticker()
{
	detach();
}

void Ticker::launch_timer(Ticker *_this)
{
	_this->run_timer();
}

void Ticker::run_timer()
{
	bool repeat = _repeat;
	do
	{
		this_thread::sleep_for(chrono::milliseconds(1000));
		_callback(reinterpret_cast<void*>(_arg));
	} while (repeat && !_stop_timer_thread);
}


void Ticker::_attach_ms(uint32_t milliseconds, bool repeat, callback_with_arg_t callback, uint32_t arg)
{
	_milliseconds = milliseconds;
	_repeat = repeat;
	_callback = callback;
	_arg = arg;

	_thread = new thread(launch_timer, this);
}

void Ticker::detach()
{
	_stop_timer_thread = true;
	_thread->join();
	delete _thread;
}
