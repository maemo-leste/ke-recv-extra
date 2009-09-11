/**
  @file hulda.h
  
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

#ifndef HULDA_H_
#define HULDA_H_

#include <stdlib.h>
#include <sys/wait.h>
#include <osso-log.h>
#include <glib-2.0/glib.h>
#include <assert.h>

#ifndef __USE_BSD
#define __USE_BSD
#define HULDA_DEFINED_USE_BSD
#endif
#include <unistd.h>
#ifdef HULDA_DEFINED_USE_BSD
#undef __USE_BSD
#endif

#include <string.h>
#include <errno.h>
#include <config.h>

#include <dbus/dbus.h>

#ifdef __cplusplus
extern "C" {
#endif

#define APPL_NAME "hulda"
#define APPL_VERSION "1"

#define SVC_NAME "org.kernel.kevent"
#define OP_NAME "/org/kernel/kevent"
#define IF_NAME SVC_NAME
#define CHANGE_SIG "change"
#define ADD_SIG "add"
#define REMOVE_SIG "remove"
#define COVER_OPEN_STR "open"  /* MMC cover open */
#define COVER_CLOSED_STR "closed"
#define DETACHED_STR "disconnected" /* USB cable detached */

#define KEVENT_MATCH_RULE "type='signal',interface='" IF_NAME "'"

/* MCE interface */
#define MCE_SERVICE "com.nokia.mce"
#define MCE_REQUEST_IF "com.nokia.mce.request"
#define MCE_REQUEST_OP "/com/nokia/mce/request"
#define MCE_SIGNAL_IF "com.nokia.mce.signal"
#define MCE_SIGNAL_OP "/com/nokia/mce/signal"
#define MCE_GET_DEVICELOCK_MSG "get_devicelock_mode"
#define MCE_DEVICELOCK_SIG "devicelock_mode_ind"
#define MCE_SHUTDOWN_SIG "shutdown_ind"
#define MCE_LOCKED_STR "locked"
#define MCE_SHUTDOWN_SIG_MATCH_RULE "type='signal',interface='" MCE_SIGNAL_IF "',member='"\
    MCE_SHUTDOWN_SIG "'"

/* low-memory signal from kdbusd */
#define LOWMEM_SIGNAL_OP "/org/kernel/kernel/high_watermark"
#define SYSFS_LOWMEM_STATE_FILE "/sys/kernel/high_watermark"

/* background killing signal */
#define BGKILL_SIGNAL_OP "/org/kernel/kernel/low_watermark"
#define SYSFS_BGKILL_STATE_FILE "/sys/kernel/low_watermark"

#define TN_BGKILL_ON_SIGNAL_NAME "bgkill_on"
#define TN_BGKILL_ON_SIGNAL_IF "com.nokia.ke_recv.bgkill_on"
#define TN_BGKILL_ON_SIGNAL_OP "/com/nokia/ke_recv/bgkill_on"
#define TN_BGKILL_OFF_SIGNAL_NAME "bgkill_off"
#define TN_BGKILL_OFF_SIGNAL_IF "com.nokia.ke_recv.bgkill_off"
#define TN_BGKILL_OFF_SIGNAL_OP "/com/nokia/ke_recv/bgkill_off"

/* lowmem signals */
#define LOWMEM_ON_SIGNAL_NAME "lowmem_on"
#define LOWMEM_ON_SIGNAL_IF "com.nokia.ke_recv.lowmem_on"
#define LOWMEM_ON_SIGNAL_OP "/com/nokia/ke_recv/lowmem_on"
#define LOWMEM_OFF_SIGNAL_NAME "lowmem_off"
#define LOWMEM_OFF_SIGNAL_IF "com.nokia.ke_recv.lowmem_off"
#define LOWMEM_OFF_SIGNAL_OP "/com/nokia/ke_recv/lowmem_off"

/* user lowmem signal */
#define USER_LOWMEM_OFF_SIGNAL_OP "/com/nokia/ke_recv/user_lowmem_off"
#define USER_LOWMEM_OFF_SIGNAL_IF "com.nokia.ke_recv.user_lowmem_off"
#define USER_LOWMEM_OFF_SIGNAL_NAME "user_lowmem_off"
#define USER_LOWMEM_ON_SIGNAL_OP "/com/nokia/ke_recv/user_lowmem_on"
#define USER_LOWMEM_ON_SIGNAL_IF "com.nokia.ke_recv.user_lowmem_on"
#define USER_LOWMEM_ON_SIGNAL_NAME "user_lowmem_on"

typedef enum {
	LOWMEM_INVALID = 0,
	LOWMEM_ON,
	LOWMEM_OFF
} lowmem_state_t;

typedef enum {
	BGKILL_INVALID = 0,
	BGKILL_ON,
	BGKILL_OFF
} bgkill_state_t;

typedef enum {
	INPUT_DEVICE_INVALID = 0,
	INPUT_DEVICE_DETACHED,
	INPUT_DEVICE_ATTACHED
} input_device_t;

/* public functions */
void send_error(const char* n);
void send_reply(void);
void send_lowmem_state_on(void);
void send_lowmem_state_off(void);
void send_bgkill_on_signal(void);
void send_bgkill_off_signal(void);
void send_user_lowmem_on_signal(void);
void send_user_lowmem_off_signal(void);
void sysfs_change(const char *path, const char *value);
void handle_kevent(DBusMessage* m);
void send_systembus_signal(const char *op, const char *iface,
                                           const char *name);

#ifdef __cplusplus
}
#endif
#endif /* HULDA_H_ */
