This folder contain a dev-cpp project to compile for windows.

The prefered way is to use cygwin with the standard Makefile system, but this should work.

The things you should have to do are :

1/ Download and install the latest dev-cpp and the SDL/SDL_image/SDL_mixer packages. Everything can be found at :
http://www.bloodshed.net/devcpp.html

2/ Modify the project configuration to add the include paths to goomsl, iosfc, iosfc/win32, gametools, and finally the include/SDL folder from your dev-cpp directory.

3/ Once the Binary is built, put it in a folder along with the necessary      run-time dll (see below) and a copy of the data folder.

SDL run-time libraries can be downloaded at :
http://www.libsdl.org/download-1.2.php
http://www.libsdl.org/projects/SDL_image/
http://www.libsdl.org/projects/SDL_mixer/

4/ Launch and enjoy.
