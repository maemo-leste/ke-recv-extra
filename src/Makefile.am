sbin_PROGRAMS = hulda

INCLUDES = -DLOCALEDIR=\"$(LOCALEDIR)\"

hulda_CFLAGS= $(GLIB_CFLAGS) $(GCONF_CFLAGS) $(DBUS_GLIB_CFLAGS) $(DBUS_CFLAGS) $(OSSO_CFLAGS)
hulda_LDADD = $(GLIB_LIBS) $(GCONF_LIBS) $(DBUS_GLIB_LIBS) $(DBUS_LIBS) $(OSSO_LIBS)

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
