sbin_PROGRAMS = hulda

INCLUDES = \
	`pkg-config --cflags libosso` \
	`pkg-config --cflags glib-2.0` \
	`pkg-config --cflags gconf-2.0` \
	`pkg-config --cflags dbus-1` \
	`pkg-config --cflags dbus-glib-1` \
	-DLOCALEDIR=\"`pkg-config --variable=localedir osso-af-settings`\"

hulda_LDFLAGS = \
	`pkg-config --libs gconf-2.0 dbus-glib-1`

hulda_SOURCES = \
        hulda.h \
        events.h \
	lowmem-state.h \
	input-device.h \
	hulda.c \
	events.c \
	lowmem-state.c \
	input-device.c \
	kdbus.c
