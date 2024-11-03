#!/bin/sh -e

TARGET=PCTest
TMP=./tmp
OUT=./out
HASHLINK=./.haxelib/hashlink/git/src

# Build Hashlink bytecode
haxe -p ./src/ -m $TARGET --hl $OUT/$TARGET.hl

# Build Hashlink bytecode VM + exec
haxe -p ./src/ -m $TARGET --hl $TMP/$TARGET.c
gcc -o $OUT/$TARGET src/*.c $HASHLINK/code.c $HASHLINK/jit.c $HASHLINK/module.c \
    -I hashlink/include/ -I $HASHLINK/ -I $TMP/ \
    -L hashlink/ \
    -l hl -l m \
    -D USE_BYTECODE
