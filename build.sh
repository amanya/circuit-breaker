#!/bin/sh

cc -D CIRCUITBREAKER_SLOW=1 main.c cb_board.c cb_street.c -g $(pkg-config --libs --cflags raylib) -o main
