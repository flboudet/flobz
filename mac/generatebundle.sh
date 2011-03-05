#!/bin/sh

where_am_i=$(
if [ -e "$0" ]; then
prog=$0
else
prog=$(command -v -- "$0") || exit
fi
cd -P -- "$(dirname -- "$prog")" && pwd -P
)

SOURCE_DIRECTORY="${where_am_i}/.."
BUILD_DIRECTORY="$PWD/build_mac"
BUILD_ARCH="i386"

ADDITIONAL_FRAMEWORKS_DIRECTORY="/Users/flobo/Library/Frameworks"
ADDITIONAL_FRAMEWORKS="SDL SDL_image SDL_ttf SDL_mixer"

SDL_LIBS="-F${ADDITIONAL_FRAMEWORKS_DIRECTORY} -framework Cocoa -framework Security -framework IOKit -lssl -lcrypto"
for FWNAME in ${ADDITIONAL_FRAMEWORKS}
do
    SDL_LIBS="${SDL_LIBS} -framework ${FWNAME}"
done
SDL_CFLAGS="-I${ADDITIONAL_FRAMEWORKS_DIRECTORY}/SDL.framework/Headers ${SDL_LIBS}"
SDL_VERSION="1.2.12"

for CURARCH in ${BUILD_ARCH}
do
    ARCHFLAGS="${ARCHFLAGS} -arch ${CURARCH}"
done

mkdir -p "${BUILD_DIRECTORY}"
cd "${BUILD_DIRECTORY}"
${SOURCE_DIRECTORY}/configure --disable-network-internet -disable-dependency-tracking CXXFLAGS="${ARCHFLAGS}" CFLAGS="${ARCHFLAGS}" OBJCFLAGS="${ARCHFLAGS}" SDL_LIBS="${SDL_LIBS}" SDL_CFLAGS="${SDL_CFLAGS}" SDL_VERSION="${SDL_VERSION}" ADDITIONAL_FRAMEWORKS_DIRECTORY="${ADDITIONAL_FRAMEWORKS_DIRECTORY}" ADDITIONAL_FRAMEWORKS="${ADDITIONAL_FRAMEWORKS}"
make -j3
make -j3 mac-bundle

# Additional steps to compensate bad scripting
rm -rf FloboPop.app/Content/MacOS/fpserver
rm -rf FloboPop.app/Contents/Resources/data/complete.002
codesign -f -v -s "3rd Party Mac Developer Application: Fovea" "FloboPop.app"

#configure -disable-dependency-tracking SDL_LIBS="-F/Users/flobo/Library/Frameworks/ -framework Cocoa -framework SDL -framework SDL_image -framework SDL_ttf -framework SDL_mixer" SDL_CFLAGS="-I/Users/flobo/Library/Frameworks/SDL.framework/Headers/ -F/Users/flobo/Library/Frameworks/ -framework SDL -framework SDL_image -framework SDL_ttf -framework SDL_mixer" SDL_VERSION="1.2.12" CXXFLAGS="-arch i386 -arch ppc" CFLAGS="-arch i386 -arch ppc" OBJCFLAGS="-arch i386 -arch ppc"
