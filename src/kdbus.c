/*
 * kdbusd.c - get kevents from the kernel and send it to dbus
 *
 * Copyright (C) 2004 Kay Sievers <kay.sievers@vrfy.org>
 * Copyright (C) 2005-2007 Nokia Corporation. All rights reserved.
 *
 *
 *	This program is free software; you can redistribute it and/or modify it
 *	under the terms of the GNU General Public License as published by the
 *	Free Software Foundation version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful, but
 *	WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *	General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License along
 *	with this program; if not, write to the Free Software Foundation, Inc.,
 *	675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Modified by Kimmo Hämäläinen <kimmo.hamalainen@nokia.com>:
 * - integrated to Glib mainloop, ported to v0.60 DBus, integrated
 *   with ke-recv program, etc.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/user.h>
#include <linux/netlink.h> 
#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <glib.h>
#include <errno.h>
#include <assert.h>

#ifndef NETLINK_KOBJECT_UEVENT
#define NETLINK_KOBJECT_UEVENT 15
#endif

#include <osso-log.h>
#define dbg(format, arg...) ULOG_DEBUG_F(format, ## arg)

#define strfieldcpy(to, from) \
do { \
	to[sizeof(to)-1] = '\0'; \
	strncpy(to, from, sizeof(to)-1); \
} while (0)

#define strfieldcat(to, from) \
do { \
	to[sizeof(to)-1] = '\0'; \
	strncat(to, from, sizeof(to) - strlen(to)-1); \
} while (0)

#define MAX_OBJECT 256

void sysfs_change(const char *path, const char *value);
void handle_kevent(DBusMessage* m);

static DBusConnection* sysbus;
static int pipe_fd;

static void tidy_op_for_dbus(char* s)
{
        char* p = s;
        int i;
        for (i = 0; *(p + i) != '\0' && i < MAX_OBJECT; ++i) {
                switch (*(p + i)) {
                        case '.':
                        case '-':
                        case ':':
                                *(p + i) = '_';
                                break;
                        default:
                                break;
                }
        }
}

/* returns false on EOF or error */
static gboolean read_from_socket(GIOChannel* ch)
{
	char buf[1024];
	char object[MAX_OBJECT];
	DBusMessage* message;
	char *signal;
	char *pos;
	int sock;
        ssize_t len;

	sock = g_io_channel_unix_get_fd(ch);
read_again:
        len = recv(sock, &buf, sizeof(buf), 0);
        if (len == -1 && errno == EINTR) {
                goto read_again;
        }
        if (len <= 0) {
		ULOG_ERR_F("socket disconnected or error");
		return FALSE;
	}

	buf[len] = '\0';

	/* sending object */
	pos = strchr(buf, '@');
	pos[0] = '\0';
	strfieldcpy(object, "/org/kernel");
	strfieldcat(object, &pos[1]);

	/* signal emitted from object */
	signal = buf;

	/* dbg("'%s' from '%s'", signal, object); */

	/*
	 * path (emitting object)
	 * interface (type of object)
	 * name (of signal)
	 */
        tidy_op_for_dbus(object);
	message = dbus_message_new_signal(object, "org.kernel.kevent",
					  signal);

	if (message == NULL) {
		dbg("error allocating message");
		return TRUE;
	}

        /* let ke-recv handle it before sending it */
        handle_kevent(message);

	if (!dbus_connection_send(sysbus, message, NULL))
		dbg("error sending d-bus message");

	dbus_message_unref(message);
	dbus_connection_flush(sysbus);
        return TRUE;
}

#define PIPE_MSG_LEN 512

/* returns false on EOF or error */
static gboolean read_from_pipe(GIOChannel* ch)
{
	char buf[PIPE_MSG_LEN];
	char *pos;
	int fd;
        ssize_t len;

	fd = g_io_channel_unix_get_fd(ch);

        /* TODO: ensure that the whole message is read */
read_again:
        len = read(fd, buf, PIPE_MSG_LEN);
        if (len == -1 && errno == EINTR) {
                goto read_again;
        }
        if (len <= 0) {
                ULOG_ERR_F("read() failed: %s", strerror(errno));
                return FALSE;
        }
	ULOG_DEBUG_F("from pipe: '%s'", buf);
        assert(len == PIPE_MSG_LEN);

	/* sending object */
	pos = strchr(buf, '@');
        assert(pos != NULL);
	pos[0] = '\0';

        /* let ke-recv handle it */
        sysfs_change(pos + 1, buf);

        return TRUE;
}

static GIOChannel *pipe_channel;

static gboolean
kdbus_sock_cb(GIOChannel* ch, GIOCondition cond, gpointer not_used)
{
        switch (cond) {
                case G_IO_ERR:
                        ULOG_ERR_F("error for FD %d",
                                   g_io_channel_unix_get_fd(ch));
                        return FALSE;
                case G_IO_HUP:
                        ULOG_ERR_F("SIGHUP from FD %d",
                                   g_io_channel_unix_get_fd(ch));
                        return FALSE;
                case G_IO_IN:
                        if (ch == pipe_channel) {
                                return read_from_pipe(ch);
                        } else {
                                return read_from_socket(ch);
                        }
                default:
                        ULOG_ERR_F("unknown GIOCondition %d for FD %d",
                                   cond, g_io_channel_unix_get_fd(ch));
                        break;
        }
        return TRUE;
}

static gboolean
sysfs_file_cb(GIOChannel* ch, GIOCondition cond, gpointer data)
{
        GIOStatus ret;
        gchar *str = NULL;
        GError *error = NULL;
        gsize len;
        const char *file = data;

        if (cond & G_IO_IN || cond & G_IO_PRI) {
                ret = g_io_channel_read_line(ch, &str, &len, NULL, &error);
                if (error != NULL) {
                        ULOG_ERR_F("g_io_channel_read_line(): %s",
                                   error->message);
                        g_error_free(error);
                        g_io_channel_unref(ch);
                        return FALSE;
                }
                if (ret == G_IO_STATUS_NORMAL) {
                        char buf[PIPE_MSG_LEN];
                        int n;

                        str = g_strchomp(str);
                        ULOG_DEBUG_F("%s: '%s'", file, str);
                        /* make the data look like a kevent message */
                        n = snprintf(buf, PIPE_MSG_LEN, "%s@%s", str, file);
                        g_free(str);
                        if (n >= PIPE_MSG_LEN) {
                                ULOG_ERR_F("snprintf could not fit everything");
                                return FALSE;
                        } else if (n < 0) {
                                ULOG_ERR_F("snprintf error");
                                return FALSE;
                        }
                        /* zero rest of the buffer */
                        memset(buf + n, 0, PIPE_MSG_LEN - n);
                        /* write it to the pipe */
write_again:
                        n = write(pipe_fd, buf, PIPE_MSG_LEN);
                        if (n == -1 && errno == EINTR) {
                                goto write_again;
                        } else if (n == -1 && errno == EPIPE) {
                                ULOG_CRIT_F("reading end was closed");
                                exit(1);
                        } else if (n == -1) {
                                ULOG_ERR_F("write() failed: %s",
                                           strerror(errno));
                                return FALSE;
                        }
                        assert(n == PIPE_MSG_LEN);
                } 

                /* seek to zero offset so that the poll() works */
                g_io_channel_seek_position(ch, 0, G_SEEK_SET, &error);
                if (error != NULL) {
                        ULOG_ERR_F("g_io_channel_seek_position(): %s",
                                   error->message);
                        g_error_free(error);
                        g_io_channel_unref(ch);
                        return FALSE;
                }
                return TRUE;
        } else if (cond & G_IO_ERR) {
                ULOG_ERR_F("file error for %s", file);
                g_io_channel_unref(ch);
                exit(1);
        } else {
                ULOG_ERR_F("unknown GIOCondition: %d", cond);
                g_io_channel_unref(ch);
        }
        return FALSE;
}

void setup_sysfs_poll(const char *file, int pipefd);
void setup_sysfs_poll(const char *file, int pipefd)
{
        GIOChannel *gioch;
        GError *error = NULL;
        gsize len;
        gchar *dump = NULL;
        GIOStatus ret;

        gioch = g_io_channel_new_file(file, "r", &error);
        if (gioch == NULL) {
                ULOG_ERR_F("g_io_channel_new_file() for %s failed: %s",
                           file, error->message);
                g_error_free(error);
                return;
        }
#if 0
        ret = g_io_channel_set_encoding(gioch, NULL, &error);
        if (error != NULL) {
                ULOG_ERR_F("g_io_channel_set_encoding(): %s",
                           error->message);
                g_error_free(error);
                error = NULL;
        }
	g_io_channel_set_buffered(gioch, FALSE);
#endif

        /* Have to read the contents even though it's not used */
        ret = g_io_channel_read_to_end(gioch, &dump, &len, &error);
        if (ret == G_IO_STATUS_NORMAL) {
                g_free(dump);
        }
        if (error != NULL) {
                ULOG_ERR_F("g_io_channel_read_to_end(): %s",
                           error->message);
                g_error_free(error);
        }

        g_io_channel_seek_position(gioch, 0, G_SEEK_SET, &error);
        if (error != NULL) {
                ULOG_ERR_F("g_io_channel_seek_position(): %s",
                           error->message);
                g_error_free(error);
        }

        pipe_fd = pipefd;
        g_io_add_watch(gioch, G_IO_IN | G_IO_PRI | G_IO_ERR,
                       sysfs_file_cb, (gpointer)file);
}

void kdbus_init(DBusConnection* sysbus_connection, int pipefd);
void kdbus_init(DBusConnection* sysbus_connection, int pipefd)
{
	int sock;
	struct sockaddr_nl snl;
	int retval;
        GIOChannel* gioch;
        GIOStatus rc;
        GError* gerr = NULL;
	DBusError error;

	if (getuid() != 0) {
		dbg("need to be root, exit");
		exit(1);
	}
        sysbus = sysbus_connection;

	memset(&snl, 0x00, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 0xffff;

	sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (sock == -1) {
		dbg("error getting socket, exit");
		exit(1);
	}

	retval = bind(sock, (struct sockaddr *) &snl,
		      sizeof(struct sockaddr_nl));
	if (retval < 0) {
		dbg("bind failed, exit");
		exit(1);
	}

	dbus_error_init(&error);
	dbus_bus_request_name(sysbus, "org.kernel", 0, &error);
	if (dbus_error_is_set(&error)) {
		dbg("cannot acquire service, error %s: %s'",
		       error.name, error.message);
		exit(1);
	}

        gioch = g_io_channel_unix_new(sock);
        if (gioch == NULL) {
                ULOG_ERR_F("g_io_channel_unix_new() for FD %d failed",
                           sock);
                return;
        }
        rc = g_io_channel_set_encoding(gioch, NULL, &gerr);
        if (rc != G_IO_STATUS_NORMAL) {
                ULOG_ERR_F("failed to set the encoding: %s", gerr->message);
                exit(1);
        }
	g_io_channel_set_buffered(gioch, FALSE);
        g_io_add_watch(gioch, G_IO_IN | G_IO_ERR | G_IO_HUP,
                       kdbus_sock_cb, NULL);

        /* set up reading from the pipe */
        pipe_channel = g_io_channel_unix_new(pipefd);
        if (pipe_channel == NULL) {
                ULOG_ERR_F("g_io_channel_unix_new() for FD %d failed",
                           pipefd);
                return;
        }
        rc = g_io_channel_set_encoding(pipe_channel, NULL, &gerr);
        if (rc != G_IO_STATUS_NORMAL) {
                ULOG_ERR_F("failed to set the encoding: %s", gerr->message);
                exit(1);
        }
	g_io_channel_set_buffered(pipe_channel, FALSE);
        g_io_add_watch(pipe_channel, G_IO_IN | G_IO_ERR | G_IO_HUP,
                       kdbus_sock_cb, NULL);
}

