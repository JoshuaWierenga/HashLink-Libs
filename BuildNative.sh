#!/bin/sh -e

TARGET=PCTest
TMP=./tmp
OUT=./out

haxe -p ./src/ -m $TARGET --hl $TMP/$TARGET.c
gcc -O3 -c -o $TMP/$TARGET.o -I src/ -I hashlink/include/ -I $TMP/ $TMP/$TARGET.c
ar rcs $TMP/lib$TARGET.lib $TMP/$TARGET.o
mkdir -p $OUT/
gcc -o $OUT/$TARGET src/$TARGET.c \
    -I hashlink/include/ -I $TMP/ \
    -L $TMP/ -L hashlink/ \
    -l:lib$TARGET.lib -l hl
