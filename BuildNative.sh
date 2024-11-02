# /bin/sh

TARGET=PCTest
TMP=./tmp/

haxe --main $TARGET --hl $TMP/$TARGET.c -dce std
gcc -O3 -c -o $TMP/$TARGET.o -I . -I hashlink/include/ -I $TMP $TMP/$TARGET.c
ar rcs $TMP/lib$TARGET.lib $TMP/$TARGET.o
gcc -o $TARGET $TARGET.c -I hashlink/include/ -I $TMP/ -L $TMP/ -l:lib$TARGET.lib -L hashlink/ -l hl
./$TARGET
