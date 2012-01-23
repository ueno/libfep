#!/bin/sh

gnulib-tool --libtool --import byteswap forkpty stdbool stdint wcswidth
autoreconf -f -i
