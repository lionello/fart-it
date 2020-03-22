#!/usr/bin/env sh
if [ "$CC" == "" ]; then CC=gcc ; fi
$CC fart.cpp fart_shared.c wildmat.c -o fart "$@"
