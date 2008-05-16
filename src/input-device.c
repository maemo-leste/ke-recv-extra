/**
  @file input-device.c
      
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

#include "hulda.h"
#include "input-device.h"

static input_device_t input_device_state = INPUT_DEVICE_INVALID;
static gint num_devices;

input_device_t get_input_device_state(void)
{
        return input_device_state;
}

void reread_input_device_state(void)
{
        gchar *path;
        gint i;

        input_device_state = INPUT_DEVICE_DETACHED;
        num_devices = 0;

        /* If an input device is already plugged in, path will exist */
        for (i = 1; i < INPUT_DEVICE_MAX_DEVICES; i++) {
                path = g_strdup_printf("%s%d",
                                       INPUT_DEVICE_SYSFS_FILE,
                                       INPUT_DEVICE_BUILTIN_EVENTS + i);

                if (g_file_test(path, G_FILE_TEST_IS_DIR)) {
                        input_device_ref();
                }

                g_free(path);
        }
}

void init_input_device_state(void)
{
        assert(input_device_state == INPUT_DEVICE_INVALID);
        reread_input_device_state();
}

void input_device_ref(void)
{
        ++num_devices;
        input_device_state = INPUT_DEVICE_ATTACHED;
}

void input_device_unref(void)
{
        g_return_if_fail(num_devices > 0);
        --num_devices;
        if (num_devices == 0) {
                input_device_state = INPUT_DEVICE_DETACHED;
        }
}
