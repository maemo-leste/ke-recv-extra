/**
  @file hulda.c

  This file is part of ke-recv-extra.

  Copyright (C) 2004-2009 Nokia Corporation. All rights reserved.

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

#include <libintl.h>
#include <locale.h>
#include <config.h>
#include "hulda.h"
#include "events.h"
#include <dbus/dbus-glib-lowlevel.h>

const char* sysfs_lowmem_file = NULL;
const char* sysfs_bgkill_file = NULL;

void kdbus_init(DBusConnection* sysbus_connection, int fd);
void setup_sysfs_poll(const char *file, int fd);

/* "the connection" is the connection where "the message" (i.e.
   the current message or signal that we're handling) came */
static DBusConnection* the_connection = NULL;
static DBusConnection* sys_conn = NULL, *ses_conn = NULL;
static DBusMessage* the_message = NULL;

void send_error(const char* s);
void show_infonote(const char *text);

/* strcmp() with checks for NULL pointers */
static int strcmp2(const char* s1, const char* s2)
{
    if (s1 == NULL || s2 == NULL) {
        return -1;
    }
    return strcmp(s1, s2);
}

void sysfs_change(const char *path, const char *value)
{
    the_connection = sys_conn;
    the_message = NULL; /* replying to message 'm' wouldn't work */

    if (strcmp2(path, sysfs_lowmem_file) == 0) {
        lowmem_state_t old, new;
        old = get_lowmem_state();
        assert(old != LOWMEM_INVALID);
        set_lowmem_state(value);
        new = get_lowmem_state();
        /* send signal only if the state changed */
        if (new != old) {
            if (new == LOWMEM_ON) { 
                handle_event(E_LOWMEM_ON_SIGNAL);
            } else {
                handle_event(E_LOWMEM_OFF_SIGNAL);
            }
        }
    } else if (strcmp2(path, sysfs_bgkill_file) == 0) {
        bgkill_state_t old, new;
        old = get_bgkill_state();
        assert(old != BGKILL_INVALID);
        set_bgkill_state(value);
        new = get_bgkill_state();
        /* send signal only if the state changed */
        if (new != old) {
            if (new == BGKILL_ON) {
                handle_event(E_BGKILL_ON_SIGNAL);
            } else {
                handle_event(E_BGKILL_OFF_SIGNAL);
            }
        }
    }
#if 0
    else if (strcmp2(path, "/sys/devices/platform/musb_hdrc/usb1/otg_last_error") == 0) {
        if (strncmp(value, "OTG01", 5) == 0) {
            show_infoprint(gettext("stab_me_usb_otg_not_supported"));
        } else if (strncmp(value, "OTG02", 5) == 0) {
            show_infoprint(gettext("stab_me_usb_otg_not_responding"));
        } else if (strncmp(value, "OTG03", 5) == 0) {
            show_infoprint(gettext("stab_me_usb_otg_hub_support"));
        } else if (strncmp(value, "OTG04", 5) == 0) {
            show_infoprint(gettext("stab_me_usb_cannot_connect"));
        }
    }
#endif
    /* invalidate */
    the_connection = NULL;
}

#define FDO_SERVICE "org.freedesktop.Notifications"
#define FDO_OBJECT_PATH "/org/freedesktop/Notifications"
#define FDO_INTERFACE "org.freedesktop.Notifications"

void show_infonote(const char *text)
{
    DBusMessage* m = NULL;
    DBusError err;
    dbus_bool_t ret;
    char *btext = "";
    int type = 0;

    dbus_error_init(&err);
    if (!ses_conn) {
        ses_conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
        if (!ses_conn) {
                ULOG_ERR_F("couldn't open session bus");
                return;
        }
    }

    m = dbus_message_new_method_call(FDO_SERVICE, FDO_OBJECT_PATH,
                                     FDO_INTERFACE, "SystemNoteDialog");
    if (m == NULL) {
        ULOG_ERR_F("couldn't create message");
        return;
    }

    ret = dbus_message_append_args(m, DBUS_TYPE_STRING, &text,
                                   DBUS_TYPE_UINT32, &type,
                                   DBUS_TYPE_STRING, &btext,
                                   DBUS_TYPE_INVALID);
    if (!ret) {
        ULOG_ERR_F("couldn't append arguments");
        dbus_message_unref(m);
        return;
    }

    ret = dbus_connection_send(ses_conn, m, NULL);
    dbus_message_unref(m);
    if (!ret) {
        ULOG_ERR_F("dbus_connection_send failed");
        return;
    }
    dbus_connection_flush(ses_conn);
}

/* this callback is called directly from kdbus code, the message
 * does not come from the DBus bus */
void handle_kevent(DBusMessage* m)
{
#if 0  /* nothing done here because input device tracking is not needed */
        /*
    ULOG_DEBUG_F("i|m|p: %s|%s|%s",
                 dbus_message_get_interface(m), 
                 dbus_message_get_member(m),
                 dbus_message_get_path(m));
                 */
    the_connection = sys_conn;
    the_message = NULL; /* replying to message 'm' wouldn't work */

    /* Change signals are handled now in sysfs_change() and using
     * the polling way, not kevents (UEvents). */

    if (dbus_message_is_signal(m, IF_NAME, ADD_SIG)) {
        const char* path = dbus_message_get_path(m);

        if (strncmp(path, INPUT_DEVICE_OP,
                         strlen(INPUT_DEVICE_OP)) == 0) {
                input_device_ref();
                handle_event(E_INPUT_DEVICE_ATTACHED);
        }
    } else if (dbus_message_is_signal(m, IF_NAME, REMOVE_SIG)) {
        const char* path = dbus_message_get_path(m);

        if (strncmp(path, INPUT_DEVICE_OP, strlen(INPUT_DEVICE_OP)) == 0) {
                input_device_unref();
                handle_event(E_INPUT_DEVICE_DETACHED);
        }
    }
    /* invalidate */
    the_connection = NULL;
#endif
}

/**
  D-BUS message handler for others than kernel events
*/
static
DBusHandlerResult sig_handler(DBusConnection *c, DBusMessage *m,
		              void *data)
{
    gboolean handled = FALSE;
    /*
    ULOG_DEBUG_L("i|m|p: %s|%s|%s",
               dbus_message_get_interface(m), 
               dbus_message_get_member(m),
               dbus_message_get_path(m));
               */
    the_connection = c;
    the_message = m;
    if (dbus_message_is_signal(m, DBUS_PATH_LOCAL, "Disconnected")) {
        ULOG_WARN_L("D-Bus system bus disconnected, going to S_SHUTDOWN");
	handle_event(E_SHUTDOWN);
        handled = TRUE;
    } else if (dbus_message_is_signal(m, MCE_SIGNAL_IF,
                                      MCE_SHUTDOWN_SIG)) {
        handle_event(E_SHUTDOWN);
        handled = TRUE;
    }
    /* invalidate */
    the_connection = NULL;
    the_message = NULL;
    if (handled) {
        return DBUS_HANDLER_RESULT_HANDLED;
    } else {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
}

void send_error(const char* n)
{
        DBusMessage* e = NULL;
        assert(the_connection != NULL && the_message != NULL
                && n != NULL);
        e = dbus_message_new_error(the_message,
                        "com.nokia.ke_recv.error", n);
        if (e == NULL) {
                ULOG_ERR_F("couldn't create error");
                return;
        }
        if (!dbus_connection_send(the_connection, e, NULL)) {
                ULOG_ERR_F("sending failed");
        }
        dbus_message_unref(e);
}

void send_reply(void)
{
	DBusMessage* e = NULL;
	assert(the_connection != NULL && the_message != NULL);
	e = dbus_message_new_method_return(the_message);
	if (e == NULL) {
		ULOG_ERR_F("couldn't create reply");
		return;
	}
	if (!dbus_connection_send(the_connection, e, NULL)) {
		ULOG_ERR_F("sending failed");
	}
        dbus_message_unref(e);
}

void send_systembus_signal(const char *op, const char *iface,
                           const char *name)
{
        DBusMessage* m;
	assert(sys_conn != NULL);
        m = dbus_message_new_signal(op, iface, name);
        if (m == NULL) {
                ULOG_ERR_F("couldn't create signal %s", name);
                return;
        }
        if (!dbus_connection_send(sys_conn, m, NULL)) {
                ULOG_ERR_F("sending signal %s failed", name);
        }
        dbus_message_unref(m);
}

void send_bgkill_on_signal(void)
{
        send_systembus_signal(TN_BGKILL_ON_SIGNAL_OP,
                              TN_BGKILL_ON_SIGNAL_IF,
                              TN_BGKILL_ON_SIGNAL_NAME);
}

void send_bgkill_off_signal(void)
{
        send_systembus_signal(TN_BGKILL_OFF_SIGNAL_OP,
                              TN_BGKILL_OFF_SIGNAL_IF,
                              TN_BGKILL_OFF_SIGNAL_NAME);
}

void send_lowmem_state_on(void)
{
        send_systembus_signal(LOWMEM_ON_SIGNAL_OP,
                              LOWMEM_ON_SIGNAL_IF,
                              LOWMEM_ON_SIGNAL_NAME);
}

void send_lowmem_state_off(void)
{
        send_systembus_signal(LOWMEM_OFF_SIGNAL_OP,
                              LOWMEM_OFF_SIGNAL_IF,
                              LOWMEM_OFF_SIGNAL_NAME);
}

void send_user_lowmem_on_signal(void)
{
        send_systembus_signal(USER_LOWMEM_ON_SIGNAL_OP,
                              USER_LOWMEM_ON_SIGNAL_IF,
                              USER_LOWMEM_ON_SIGNAL_NAME);
}

void send_user_lowmem_off_signal(void)
{
        send_systembus_signal(USER_LOWMEM_OFF_SIGNAL_OP,
                              USER_LOWMEM_OFF_SIGNAL_IF,
                              USER_LOWMEM_OFF_SIGNAL_NAME);
}

static void read_config()
{
        sysfs_lowmem_file = SYSFS_LOWMEM_STATE_FILE;
        sysfs_bgkill_file = SYSFS_BGKILL_STATE_FILE;
}

static GMainLoop *mainloop = NULL;
static pid_t child_pid;

static void sigchld(int signo)
{
    int status;

    wait(&status);
    if (WIFEXITED(status)) {
            /*
        printf("child exited with code %d, exiting\n",
                    WEXITSTATUS(status));
                    */
    } else if (WIFSIGNALED(status)) {
            /*
        printf("child exited with signal %d, exiting\n",
                    WTERMSIG(status));
                    */
    }

    exit(1);
}

static void sigterm(int signo)
{
    ULOG_INFO_L("got SIGTERM, kill the child %d", child_pid);
    kill(child_pid, SIGTERM);
    exit(0);
}

/* Does initialisations and goes to the Glib main loop. */
int main(int argc, char* argv[])
{
    int pipefd[2];

    read_config();

    if (pipe(pipefd) == -1) {
      ULOG_CRIT_L("pipe() failed: %s", strerror(errno));
      exit(1);
    }

    if (signal(SIGCHLD, sigchld) == SIG_ERR) {
      ULOG_CRIT_L("signal() failed");
      exit(1);
    }

    if (setlocale(LC_ALL, "") == NULL) {
        ULOG_ERR_L("couldn't set locale");
    }
    if (bindtextdomain("hildon-status-bar-usb", LOCALEDIR) == NULL) {
        ULOG_ERR_L("bindtextdomain() failed");
    }
    if (textdomain("hildon-status-bar-usb") == NULL) {
        ULOG_ERR_L("textdomain() failed");
    }

    if ((child_pid = fork()) == -1) {
      ULOG_CRIT_L("fork() failed: %s", strerror(errno));
      exit(1);
    } else if (child_pid != 0) {
      /* the parent */
#if !GLIB_CHECK_VERSION(2,35,0)
      g_type_init();
#endif
      mainloop = g_main_loop_new(NULL, TRUE);
      ULOG_OPEN(APPL_NAME);
      close(pipefd[1]); /* close writing end */
      if (signal(SIGTERM, sigterm) == SIG_ERR) {
        ULOG_CRIT_L("signal() failed");
        exit(1);
      }
    } else {
      /* the child process */
      close(STDIN_FILENO);
      close(STDOUT_FILENO);
#ifndef OSSOLOG_STDERR
      close(STDERR_FILENO);
#endif
      mainloop = g_main_loop_new(NULL, TRUE);
      ULOG_OPEN(APPL_NAME "-child");
      close(pipefd[0]);

      /* Set up polling for certain sysfs files */
      if (sysfs_lowmem_file != NULL) {
        setup_sysfs_poll(sysfs_lowmem_file, pipefd[1]);
      }
      if (sysfs_bgkill_file != NULL) {
        setup_sysfs_poll(sysfs_bgkill_file, pipefd[1]);
      }
      setup_sysfs_poll("/proc/mounts", pipefd[1]);

      g_main_loop_run(mainloop); 
      ULOG_DEBUG_L("Returned from the main loop");
      exit(0);
    }

    {
      DBusError error;
      DBusConnection *conn = NULL;

      dbus_error_init(&error);
      conn = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
      if (conn == NULL) {
	      ULOG_CRIT_L("Failed to get system bus connection");
	      exit(1);
      }
      sys_conn = conn;
      /* sesison bus is needed for the FDO infobanners */
      ses_conn = dbus_bus_get(DBUS_BUS_SESSION, &error);
      if (ses_conn == NULL) {
	      ULOG_CRIT_L("Failed to get session bus connection");
	      exit(1);
      }
      if (!dbus_connection_add_filter(conn, sig_handler, NULL, NULL)) {
        ULOG_CRIT_L("Failed to register signal handler callback");
	exit(1);
      }

      dbus_bus_add_match(conn, MCE_SHUTDOWN_SIG_MATCH_RULE, &error);
      if (dbus_error_is_set(&error)) {
        ULOG_CRIT_L("dbus_bus_add_match for %s failed",
                    MCE_SHUTDOWN_SIG_MATCH_RULE);
	exit(1);
      }
      dbus_connection_setup_with_g_main(sys_conn, NULL);
      dbus_connection_setup_with_g_main(ses_conn, NULL);
    }
    /* initialise kevent reading (needs root)
    * and prepare the pipe for reading */
    kdbus_init(sys_conn, pipefd[0]);

    handle_event(E_STARTUP);

    g_main_loop_run(mainloop); 
    ULOG_DEBUG_L("Returned from the main loop");
    
    exit(0);
}
