/**
  @file events.c
  Event handling functions.
  
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

#include "hulda.h"
#include "events.h"

static gboolean global_init_done = FALSE;
static GConfClient* gconfclient;

static void inform_keyboard_attached(gboolean value)
{
        GError* err = NULL;
        assert(gconfclient != NULL);
        if (!gconf_client_set_bool(gconfclient, KEYBOARD_ATTACHED_KEY,
                                   value, &err) && err != NULL) {
                ULOG_ERR_F("gconf_client_set_bool(%d) failed: %s",
                           value, err->message);
                g_error_free(err);
        }
}

static void do_global_init()
{
        gconfclient = gconf_client_get_default();

        init_lowmem_state();
        init_bgkill_state();
        init_input_device_state();

        if (get_input_device_state() == INPUT_DEVICE_ATTACHED) {
                inform_keyboard_attached(TRUE);
        } else {
                inform_keyboard_attached(FALSE);
	}
}

static void handle_e_startup()
{
        if (!global_init_done) {
                do_global_init();
                global_init_done = TRUE;
        }
}

static void handle_e_shutdown()
{
}

static void handle_e_lowmem_on_signal()
{
        ULOG_DEBUG_F("sending lowmem_on");
        send_lowmem_state_on();
        send_user_lowmem_on_signal();
}

static void handle_e_lowmem_off_signal()
{
        ULOG_DEBUG_F("sending lowmem_off");
        send_lowmem_state_off();
        send_user_lowmem_off_signal();
}

static void handle_e_bgkill_on_signal()
{
        ULOG_DEBUG_F("sending bgkill_on");
        send_bgkill_on_signal();
}

static void handle_e_bgkill_off_signal()
{
        ULOG_DEBUG_F("sending bgkill_off");
        send_bgkill_off_signal();
}

static void handle_e_input_device_attached(void)
{      
        inform_keyboard_attached(TRUE);
}
 
static void handle_e_input_device_detached(void)
{
        if (get_input_device_state() != INPUT_DEVICE_ATTACHED) {
               inform_keyboard_attached(FALSE);
        }
}

void handle_event(mmc_event_t e)
{
    switch (e) {
    case E_STARTUP:
            handle_e_startup();
	    break;
    case E_SHUTDOWN:
	    handle_e_shutdown();
	    break;
    case E_LOWMEM_ON_SIGNAL:
            handle_e_lowmem_on_signal();
            break;
    case E_LOWMEM_OFF_SIGNAL:
            handle_e_lowmem_off_signal();
            break;
    case E_BGKILL_ON_SIGNAL:
            handle_e_bgkill_on_signal();
            break;
    case E_BGKILL_OFF_SIGNAL:
            handle_e_bgkill_off_signal();
            break;
    case E_INPUT_DEVICE_ATTACHED:
            handle_e_input_device_attached();
            break;
    case E_INPUT_DEVICE_DETACHED:
            handle_e_input_device_detached();
            break;
    default:
	    ULOG_WARN_F("Unknown event %d, shouldn't ever happen", e);
            break;
    }
}
