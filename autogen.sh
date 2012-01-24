#!/bin/sh

gnulib-tool --libtool --quiet --import \
    byteswap \
    forkpty \
    stdbool \
    stdint \
    wcsrtombs \
    mbsrtowcs \
    wcswidth \
    strndup || exit 1

gtkdocize || exit 1

autoreconf -f -i || exit 1
