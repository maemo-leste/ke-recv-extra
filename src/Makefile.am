sbin_PROGRAMS = hulda

hulda_CFLAGS = \
	$(GCONF_CFLAGS) $(DBUSGLIB_CFLAGS) $(DBUS_CFLAGS) \
	$(GLIB_CFLAGS) $(LIBOSSO_CFLAGS) \
	-DLOCALEDIR=\"`pkg-config --variable=localedir osso-af-settings`\"

hulda_LDFLAGS = \
	$(GCONF_LIBS) $(DBUSGLIB_LIBS) $(DBUS_LIBS) \
	$(GLIB_LIBS) $(LIBOSSO_LIBS)

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
