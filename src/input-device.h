/**
  @file input-device.h

  This file is part of ke-recv.

  Copyright (C) 2004-2006 Nokia Corporation. All rights reserved.

  Contact: Kimmo Hämäläinen <kimmo.hamalainen@nokia.com>

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

#ifndef INPUT_DEVICE_H_
#define INPUT_DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define INPUT_DEVICE_OP "/org/kernel/class/input/input"
#define INPUT_DEVICE_SYSFS_FILE "/sys/class/input/event"
#define INPUT_DEVICE_BUILTIN_EVENTS 3
#define INPUT_DEVICE_MAX_DEVICES 4

/**
  Returns state of the input devices.
  @return state of the input devicese
*/
input_device_t get_input_device_state(void);

/**
 *  * Re-reads state file.
 *   * */
void reread_input_device_state(void);

/**
  Initialises the input device state.
*/
void init_input_device_state(void);

/**
  Increase input device reference count.
*/
void input_device_ref(void);

/**
  Decrease input device reference count.
*/
void input_device_unref(void);

#ifdef __cplusplus
}
#endif
#endif /* INPUT_DEVICE_H_ */
