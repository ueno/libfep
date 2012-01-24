#!/bin/sh

gnulib-tool --libtool --import \
    byteswap \
    forkpty \
    stdbool \
    stdint \
    wcsrtombs \
    mbsrtowcs \
    wcswidth \
    strndup

autoreconf -f -i
