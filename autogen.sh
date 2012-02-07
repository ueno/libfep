#!/bin/sh

gnulib-tool --libtool --quiet --import \
    byteswap \
    forkpty \
    stdbool \
    stdint \
    wcsrtombs \
    mbsrtowcs \
    wcswidth \
    striconv \
    nl_langinfo \
    strndup || exit 1

gtkdocize || exit 1

autoreconf -f -i || exit 1
