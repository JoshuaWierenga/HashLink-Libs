# /bin/sh

TARGET=PCTest
TMP=./tmp/

haxe --main $TARGET --hl $TARGET.hl -dce std
haxe --main $TARGET --hl $TMP/$TARGET.c -dce std
gcc -o $TARGET $TARGET.c hlc_main.c hashlink/*.c -I hashlink/include -I $TMP -L hashlink -l hl -l m -D USE_BYTECODE
./$TARGET
