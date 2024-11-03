# Build instructions
Download HashLink, Haxe and Neko binaries to subdirectories with matching lowercase names.
If they are not already installed then run:
```sh
./Setup.sh
```
which will install them and then launch a subshell with them available.

Set haxelib library download location with:
```sh
haxelib setup
```
and enter `haxelib` as the path.

Download newest hashlink repo with:
```sh
haxelib git hashlink https://github.com/HaxeFoundation/hashlink.git master other/haxelib/
```

Now to build and run the test HashLink bytecode program that runs the PCTest.test and PCTest2.test functions from Haxe:
```sh
./BuildByteCode
./PCTest
```

Now to build and run the test HashLink/C program that runs the PCTest.test and PCTest2.test functions from Haxe:
```sh
./BuildNative
./PCTest
```
