/**
  @file lowmem-state.h
  Code concerning the low-memory state.

  This file is part of ke-recv.

  Copyright (C) 2004-2008 Nokia Corporation. All rights reserved.

  Author: Kimmo Hämäläinen <kimmo.hamalainen@nokia.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License 
  version 2 as published by the Free Software Foundation. 

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
  02110-1301 USA
*/

#ifndef LOWMEM_STATE_H_
#define LOWMEM_STATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
  Returns the low-memory state.
  @return the low-memory state
*/
lowmem_state_t get_lowmem_state(void);

/**
  Initialises the state.
*/
void init_lowmem_state(void);

/**
  Sets the state.
  @param s value for the state
*/
void set_lowmem_state(const char* s);

/**
  Returns the background-killing state.
  @return the background-killing state
*/
bgkill_state_t get_bgkill_state(void);

/**
  Initialises the state.
*/
void init_bgkill_state(void);

/**
  Sets the state.
  @param s value for the state
*/
void set_bgkill_state(const char* s);

void reread_bgkill_state(void);
void reread_lowmem_state(void);

#ifdef __cplusplus
}
#endif
#endif /* LOWMEM_STATE_H_ */
