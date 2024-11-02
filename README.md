# Build instructions
Download HashLink, Haxe and Neko binaries to subdirectories with matching lowercase names.

Set haxelib library download location with:
```sh
haxelib setup
```
and enter `haxelib` as the path.

Download newest hashlink repo with:
```sh
haxelib git hashlink https://github.com/HaxeFoundation/hashlink.git master other/haxelib/
```

Now to build and run the test program that runs the PCTest.test from Haxe function:
```sh
./BuildNative
./PCTest
```
