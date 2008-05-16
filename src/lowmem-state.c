/**
  @file lowmem-state.c
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

#include "hulda.h"
#include "lowmem-state.h"

static lowmem_state_t lowmem_state = LOWMEM_INVALID;
static bgkill_state_t bgkill_state = BGKILL_INVALID;

lowmem_state_t get_lowmem_state(void)
{
        return lowmem_state;
}

void reread_lowmem_state(void)
{
        gchar* buf = NULL;
        GError* err = NULL;
        g_file_get_contents(SYSFS_LOWMEM_STATE_FILE, &buf, NULL, &err);
        if (err != NULL) {
                ULOG_ERR_F("couldn't read low-memory state file %s",
                        SYSFS_LOWMEM_STATE_FILE);
                g_error_free(err);
                return;
        }
        if (strncmp(buf, "1", 1) == 0) {
                lowmem_state = LOWMEM_ON;
        } else {
                lowmem_state = LOWMEM_OFF;
        }
        g_free(buf);
}

void init_lowmem_state(void)
{
	assert(lowmem_state == LOWMEM_INVALID);
        reread_lowmem_state();
}

void set_lowmem_state(const char* s)
{
        if (*s == '1') {
	        lowmem_state = LOWMEM_ON;
        } else {
	        lowmem_state = LOWMEM_OFF;
        }
}

bgkill_state_t get_bgkill_state(void)
{
        return bgkill_state;
}

void reread_bgkill_state(void)
{
        gchar* buf = NULL;
        GError* err = NULL;
        g_file_get_contents(SYSFS_BGKILL_STATE_FILE, &buf, NULL, &err);
        if (err != NULL) {
                ULOG_ERR_F("couldn't read bgkill state file %s",
                        SYSFS_BGKILL_STATE_FILE);
                g_error_free(err);
                return;
        }
        if (strncmp(buf, "1", 1) == 0) {
                bgkill_state = BGKILL_ON;
        } else {
                bgkill_state = BGKILL_OFF;
        }
        g_free(buf);
}

void init_bgkill_state(void)
{
	assert(bgkill_state == BGKILL_INVALID);
        reread_bgkill_state();
}

void set_bgkill_state(const char* s)
{
        if (*s == '1') {
	        bgkill_state = BGKILL_ON;
        } else {
	        bgkill_state = BGKILL_OFF;
        }
}
