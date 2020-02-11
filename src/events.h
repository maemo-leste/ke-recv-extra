/**
  @file events.h
  
  This file is part of ke-recv-extra.

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

#ifndef EVENTS_H_
#define EVENTS_H_

#include <glib.h>
#include <gconf/gconf-client.h>
#include "lowmem-state.h"
#include "input-device.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KEYBOARD_ATTACHED_KEY "/system/osso/af/keyboard-attached"

#define DEV_INPUT_PATH "/dev/input"

typedef enum {
        E_STARTUP,
        E_SHUTDOWN,
        E_LOWMEM_ON_SIGNAL,
        E_LOWMEM_OFF_SIGNAL,
        E_BGKILL_ON_SIGNAL,
        E_BGKILL_OFF_SIGNAL,
        E_INPUT_DEVICE_CHANGED
} mmc_event_t;

/**
 *   This function handles all events.
 *   @param e type of event
 */
void handle_event(mmc_event_t e);

#ifdef __cplusplus
}
#endif
#endif /* EVENTS_H_ */
