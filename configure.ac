AC_INIT(src/hulda.c)
AM_INIT_AUTOMAKE(ke-recv-extra, 0.2)
AM_CONFIG_HEADER(config.h)
AC_PROG_CC

PKG_CHECK_MODULES([KERECV], [gio-2.0 gconf-2.0 dbus-glib-1 dbus-1 glib-2.0 libosso])
AC_SUBST(KERECV_CFLAGS)
AC_SUBST(KERECV_LIBS)

LOCALEDIR=`pkg-config --variable=localedir osso-af-settings`
AC_SUBST(LOCALEDIR)

AC_OUTPUT([Makefile src/Makefile])
