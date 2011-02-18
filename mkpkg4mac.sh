
DIR=$(dirname $(which $0))

BUILDDIR=$DIR/build/projects/DiveBoard/Debug/DiveBoard.plugin/Contents/MacOS
OUTDIR=$DIR/build/packages

LIBDIVE=$DIR/libdivecomputer/build/Debug/liblibdivecomputer.dylib

TMPDIR=/tmp

PKGNAME=diveboard.pkg


mkdir -p $OUTDIR



####
#### Create package of Diveboard.plugin directory
####

# Copy libdivecomputer into Diveboard.plugin directory
cp "$LIBDIVE" "$BUILDDIR"

# Create the package
/Developer/usr/bin/packagemaker --root "$DIR/build/projects/DiveBoard/Debug/DiveBoard.plugin" --install-to "/Library/Internet Plug-Ins" --out "$TMPDIR/$PKGNAME" --id com.diveboard.plugin.pkg --title "Diveboard plugin" --root-volume-only --domain system

####
#### Assemble full package
####

# Initialize the final package
rm -fr $OUTDIR/bigpack.mpkg
cp -a $DIR/drivers/mac/bigpack.mpkg $OUTDIR/
rm -fr "$OUTDIR/bigpack.mpkg/Contents/Packages/$PKGNAME"

# Place the Diveboard package into the big package
cp -a "$TMPDIR/$PKGNAME" "$OUTDIR/bigpack.mpkg/Contents/Packages/$PKGNAME"

####
#### xar-ing the archive or creating a DMG file
####

# getting the space used 
SIZE=$(($(du -ks build/packages/bigpack.mpkg.tmp/ | sed 's/[^0-9].*//') + 500 ))

# Creating a dmg image
DMGFILE=/tmp/pack.temp.dmg
DMGFINALFILE=$OUTDIR/diveboard.dmg
hdiutil create -srcfolder "$OUTDIR/bigpack.mpkg" -volname "Diveboard Plugin" -fs HFS+ -fsargs "-c c=64,a=16,e=16" -format UDRW -size ${SIZE}k "$DMGFILE"

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
hdiutil convert "$DMGFILE" -format UDZO -imagekey zlib-level=9 -o "${DMGFINALFILE}"
rm -f "$DMGFILE"


