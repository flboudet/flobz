# FlobZ

FlobZ is a tile-matching puzzle game inspired by the Japanese PuyoPuyo game.

The  goal  of  FlobZ  is to make groups of four or more Puyos (coloured bubbles) to make them explode and send bad ghost Puyos to your opponent.  You win the game if your opponent reaches the top of the board. You can play against computer or another human.
The game has nice graphics and animations as well  as  music  and sound effects.

### Version
1.0

### Installation

#### Linux

If you are getting the sources from the git repository,
you first need to generate the ./configure script.
Note that this step is not necessary when you get the source tarball.

```sh
$ cd [path-to-the-source-folder-on-your-system]
$ autoreconf -i
```

then

```sh
$ cd [path-to-the-source-folder-on-your-system]
$ ./configure
$ make -j3
$ ./flobz
```

### MacOS X

I am affraid those instructions are a bit outdated...

Install the SDL* frameworks in $(SYSTEM_ROOT)/Library/Frameworks
and edit the SYSTEM_ROOT variable in the config file to match your MacOS X SDK
If you want to build a universal binary, put the universal binaries of the SDL frameworks in a universal binary SDK of MacOS X
Then type the following lines in Terminal:
```sh
cd path-to-the-source-folder-on-your-system
./configure
make -j3 bundle
```
or make mac-package to create the full package
You should get FloboPuyo.app

### Windows

I am affraid those instructions are a bit outdated...

(Cross compile using Mac OS X or Linux)
Install all the SDL* dll somewhere (one "include" and one "lib" folder for all SDL*)
Install the cross-compiler somewhere
Then type the following lines in a terminal:
export SDL_CONFIG=<path to your win32 mingw32 sdlconfig>
./configure -prefix=/usr/local/i586-mingw32 --host=i586-mingw32msvc

### Dependancies automake

#### Linux

- libtool
- bison
- flex
- libsdl1.2-dev
- libsdl-ttf2.0-dev
- libsdl-image1.2-dev
- libsdl-mixer1.2-dev
- libmikmod-dev
