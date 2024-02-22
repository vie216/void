#!/usr/bin/sh

CFLAGS="-Wall -Wextra"
VOID_SRC="$(find src -name "*.c" -not -path "src/echo/*")"
ECHO_SRC="misc/echo.c src/term.c"

cc -o echo $CFLAGS "${@:1}" $ECHO_SRC
cc -o void $CFLAGS "${@:1}" $VOID_SRC
