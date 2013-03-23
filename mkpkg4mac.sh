#!/bin/bash

BASEDIR=/tmp/diveboard_plugin.$(date '+%Y%m%d_%H%M%S')
DIR="$BASEDIR/plugin"
PLUGINDIR="$DIR/build/projects/DiveBoard/Release/DiveBoard.plugin"
BUILDDIR="$PLUGINDIR/Contents/MacOS"
OUTDIR="$DIR/build/packages"
LIBDIVE="$DIR/build/libdivecomputer/libdivecomputer.dylib"
LOCTMPDIR=/tmp
PKGNAME=diveboard.pkg



####
#### Fetch source from git
####
echo "$BASEDIR"
mkdir -p "$BASEDIR"
cd "$BASEDIR"
git clone --depth 1 git@diveboard.plan.io:diveboard-db.git plugin
cd "$DIR"
git submodule update --init
##this one fails so we let prepmac.sh fetch boost by itself #git submodule update --recursive --init
#

VERSION=`cat "$DIR/VERSION"`


###
### Force rebuild of the software
###
rm -fr "$DIR/build"
cd "$DIR" 
firebreath/prepmac.sh projects build
cd "$DIR/libdivecomputer"
make clean
xcodebuild -configuration Release -project $DIR/build/FireBreath.xcodeproj clean


###
### Build everything
###

cd "$DIR/irda_mac" 
make

#applying patch for IrDA on Libdivecomputer (will be unpatched after...)
cd "$DIR/libdivecomputer" 
patch -N -p1 < "$DIR/irda_mac/libdivecomputer_irda_mac.1.patch"
cp "$DIR/irda_mac/irda_mac.c" "$DIR/libdivecomputer/src/"

cd "$DIR/libdivecomputer" 
autoreconf --install 
DYLD_LIBRARY_PATH="$DIR/irda_mac" CPPFLAGS="-I$DIR/irda_mac/" LDFLAGS="-L$DIR/irda_mac" LIBS="-lirda" ./configure CFLAGS='-arch i386' --target=i386 --prefix="$DIR/build/libdivecomputer/i386" && make clean all install
DYLD_LIBRARY_PATH="$DIR/irda_mac" CPPFLAGS="-I$DIR/irda_mac/" LDFLAGS="-L$DIR/irda_mac" LIBS="-lirda" ./configure CFLAGS='-arch x86_64' --target=x86_64 --prefix="$DIR/build/libdivecomputer/x86_64" && make clean all install
lipo -create "$DIR/build/libdivecomputer/"*"/lib/libdivecomputer.0.dylib" -output "$LIBDIVE"

#NOT removing patch for IrDA on Libdivecomputer
#cd "$DIR/libdivecomputer" 
#patch -R -p1 < "$DIR/irda_mac/libdivecomputer_irda_mac.1.patch"

xcodebuild -configuration Release -project $DIR/build/FireBreath.xcodeproj build

###
### Create package of Diveboard.plugin directory
###

# Copy libdivecomputer into Diveboard.plugin directory
cp "$LIBDIVE" "$BUILDDIR/liblibdivecomputer.dylib"

# Create the package
echo "Launching packagemaker"
cd $DIR
/Applications/Xcode.app/Contents/Developer/usr/bin/packagemaker --root "$PLUGINDIR" --install-to "/Library/Internet Plug-Ins" --out "$LOCTMPDIR/$PKGNAME" --id com.diveboard.plugin.pkg --title "Diveboard plugin" --root-volume-only --domain system

####
#### Assemble full package
####

# Initialize the final package
echo "Initializing the final package"
mkdir -p "$OUTDIR"
rm -fr "$OUTDIR/Diveboard.mpkg"
cp -a "$DIR/drivers/mac/Diveboard.mpkg" "$OUTDIR/Diveboard.mpkg"
rm -fr "$OUTDIR/Diveboard.mpkg/Contents/Packages/$PKGNAME"

# Place the Diveboard package into the big package
echo "Putting everything into the big package"
cp -a "$LOCTMPDIR/$PKGNAME" "$OUTDIR/Diveboard.mpkg/Contents/Packages/$PKGNAME"

####
#### xar-ing the archive or creating a DMG file
####

# getting the space used 
SIZE=$(($(du -ks "$DIR/build/packages/Diveboard.mpkg/" | sed 's/[^0-9].*//') + 500 ))

# Creating a dmg image
DMGFILE=/tmp/pack.temp.dmg
DMGFINALFILE=$OUTDIR/diveboard-plugin-$VERSION.dmg
hdiutil create -srcfolder "$OUTDIR/Diveboard.mpkg" -volname "Diveboard Plugin" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -size ${SIZE}k "$DMGFILE"

# Mounting the image
device=$(hdiutil attach -readwrite -noverify -noautoopen "$DMGFILE" | egrep '^/dev/' | sed 1q | awk '{print $1}')

#echo '
#   tell application "Finder"
#     tell disk "'Diveboard Plugin'"
#           open
#           set current view of container window to icon view
#           set toolbar visible of container window to false
#           set statusbar visible of container window to false
#           set the bounds of container window to {400, 100, 885, 430}
#           set theViewOptions to the icon view options of container window
#           set arrangement of theViewOptions to not arranged
#           set icon size of theViewOptions to 72
#           set background picture of theViewOptions to file ".background:'${backgroundPictureName}'"
#           make new alias file at container window to POSIX file "/Applications" with properties {name:"Applications"}
#           set position of item "'${applicationName}'" of container window to {100, 100}
#           set position of item "Applications" of container window to {375, 100}
#           close
#           open
#           update without registering applications
#           delay 5
#           eject
#     end tell
#   end tell
#' | osascript

chmod -Rf go-w /Volumes/"Diveboard plugin"
sync
sync
hdiutil detach ${device}
rm -f "$DMGFINALFILE"
hdiutil convert "$DMGFILE" -format UDZO -imagekey zlib-level=9 -o "${DMGFINALFILE}"
rm -f "$DMGFILE"


#if [ -d "$DIR/../diveboard-web" ] 
#then
#  zip -j "$DIR/../diveboard-web/public/plugin/latest/mac/diveboard.zip" $DMGFINALFILE
#  echo "Zip file has been updated in $DIR/../diveboard-web/public/plugin/latest/mac/diveboard.zip"
#fi

if [[ -x "$HOME/Dropbox/Plugin Builds" ]]
then
  DROPDIR="$HOME/Dropbox/Plugin Builds/V$VERSION/mac_$(date '+%Y%m%d_%H%M%S')"
  mkdir -p "$DROPDIR"
  cp -a "$DMGFINALFILE" "$DROPDIR"
fi
