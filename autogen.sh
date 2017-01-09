#!/bin/sh

set -x
#glib-gettextize --copy --force
libtoolize --automake
#intltoolize --copy --force --automake
autoreconf -v -f -i

