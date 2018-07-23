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

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/uinput.h>

#define BITS_PER_LONG			(sizeof(long) * 8)
#define NBITS(x)			((((x) - 1) / BITS_PER_LONG) + 1)
#define OFF(x)				((x) % BITS_PER_LONG)
#define BIT(x)				(1UL << OFF(x))
#define LONG(x)				((x)/BITS_PER_LONG)
#define test_bit(bit, array) ((array[LONG(bit)] >> OFF(bit)) & 1)

static input_device_t input_device_state = INPUT_DEVICE_INVALID;

static const int slide_event_types[] = {
  EV_SW,
  -1
};

static const int event_slide[] = {
  SW_KEYPAD_SLIDE,
  -1
};

static const int *const slide_event_keys[]= {
  event_slide
};

static const int keyboard_event_types[] = {
  EV_KEY,
  -1
};

static const int event_keyboard[] = {
  KEY_ENTER,
  KEY_ESC,
  -1
};

static const int *const keyboard_event_keys[]= {
  event_keyboard
};

static gboolean match_event_file_by_caps(const gchar *const filename,
                                         const int *const ev_types,
                                         const int *const ev_keys[])
{
  int ev_type, p, q;
  int version;
  unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
  int fd;
  gboolean rv = FALSE;

  if ((fd = open(filename, O_NONBLOCK | O_RDONLY)) == -1) {
    /* Ignore error */
    errno = 0;
    return 0;
  }

  /* We use this ioctl to check if this device supports the input
   * ioctl's
   */
  if (ioctl(fd, EVIOCGVERSION, &version) < 0)
    goto EXIT;

  memset(bit, 0, sizeof(bit));

  if (ioctl(fd, EVIOCGBIT(0, EV_MAX), bit[0]) < 0)
    goto EXIT;

  for (p = 0; ev_types[p] != -1; p++) {
    /* TODO: Could check that ev_types[p] is less than EV_MAX */
    ev_type = ev_types[p];

    /* event type not supported, try the next one */
    if (!test_bit(ev_type, bit[0]))
      continue;

    /* Get bits per event type */
    if (ioctl(fd, EVIOCGBIT(ev_type, KEY_MAX), bit[ev_type]) < 0)
      goto EXIT;

    for (q = 0; ev_keys[p][q] != -1; q++) {
      /* TODO: Could check that q is less than KEY_MAX */

      /* succeed if at least one match is found */
      if (test_bit(ev_keys[p][q], bit[ev_type]))
      {
        rv = TRUE;
        goto EXIT;
      }
    }
  }

EXIT:
  close(fd);

  return rv;
}

input_device_t get_input_device_state(void)
{
  return input_device_state;
}

void reread_input_device_state(void)
{
  gchar *path;
  GDir *dir;
  const gchar *name;
  gint num_keyboard_devices = 0;
  gboolean slide_device_exists = FALSE;

  input_device_state = INPUT_DEVICE_DETACHED;
  slide_device_exists = FALSE;
  num_keyboard_devices = 0;

  dir = g_dir_open(INPUT_DEVICE_DIR, 0, NULL);

  if (!dir)
    return;

  while((name = g_dir_read_name(dir))) {
    if (!g_str_has_prefix(name, "event"))
      continue;

    path = g_strconcat (INPUT_DEVICE_DIR, "/", name, NULL);

    if (match_event_file_by_caps(path, slide_event_types, slide_event_keys)) {
      slide_device_exists = TRUE;
    }

    if (match_event_file_by_caps(path, keyboard_event_types,
                                 keyboard_event_keys)) {
      num_keyboard_devices++;
    }

    g_free(path);
  }

  g_dir_close(dir);

  if (slide_device_exists)
    num_keyboard_devices--;

  if (num_keyboard_devices > 0)
    input_device_state = INPUT_DEVICE_ATTACHED;
}

void init_input_device_state(void)
{
  assert(input_device_state == INPUT_DEVICE_INVALID);
  reread_input_device_state();
}
