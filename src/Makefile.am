sbin_PROGRAMS = hulda

AM_CPPFLAGS = -DLOCALEDIR=\"$(LOCALEDIR)\"

hulda_CFLAGS = $(KERECV_CFLAGS)
hulda_LDFLAGS = $(KERECV_LIBS)

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
