#!/bin/bash

VERSION=1.1.0-alpha1

DIR=$(dirname $(readlink -f $0))

BUILDDIR=$DIR/build/projects/DiveBoard/ubuntu
OUTDIR=$DIR/build/packages

LIBDIVE=$DIR/libdivecomputer/src/.libs/libdivecomputer.so.0.0.0
LIBDIVEBOARD=$DIR/build/bin/DiveBoard/npDiveBoard.so

PKGNAME=diveboard-$VERSION.deb

CONTROL="Package:diveboard
Version: $VERSION
Section: web
Priority: optional
Architecture: i386
Depends: libc6 (>= 2.11)
Maintainer: Diveboard <support@diveboard.com>
Description: Web Browser plugin for DiveBoard:
 http://www.diveboard.com
 .
 With the DiveBoard browser plugin you can upload your dive
 profiles direct from a supported dive computer to the web.
 This plugin links with Jef Driesen's LibDiveComputer http://www.divesoftware.org/libdc/

"

#
# Put the package together
#

if [ ! -f $LIBDIVEBOARD ]; then
	echo Unable to find DiveBoard - did you build it?
	echo MISSING: $LIBDIVEDBOARD
	exit 1
fi
if [ ! -f $LIBDIVE ]; then
	echo Unable to find libdivecomputer - did you build it?
	echo MISSING: $LIBDIVE
	exit 1
fi

echo Creating build structure...
mkdir -p $OUTDIR
rm -f $OUTDIR/$PKGNAME
rm -rf $BUILDDIR
mkdir -p $BUILDDIR/usr/lib/diveboard

echo Copying package contents...
cp $LIBDIVEBOARD $BUILDDIR/usr/lib/diveboard/npDiveBoard.so
cp $LIBDIVE $BUILDDIR/usr/lib/diveboard/libdivecomputer.so

echo Stripping binaries...
strip $BUILDDIR/usr/lib/diveboard/*.so

echo Fixing permissions...
chmod 644 $BUILDDIR/usr/lib/diveboard/*.so

echo Creating control file...
mkdir -p $BUILDDIR/DEBIAN
echo -e "$CONTROL" > $BUILDDIR/DEBIAN/control

echo Creating postinst script...
cat > $BUILDDIR/DEBIAN/postinst << EOF
#!/bin/bash

set -e

BROWSERS="mozilla firefox seamonkey"

if [ "\$1" == "configure" ];then
	for BROWSER in \$BROWSERS; do
		if [ -d /usr/lib/\$BROWSER ];then
			mkdir -p /usr/lib/\$BROWSER/plugins
			ln -sf /usr/lib/diveboard/npDiveBoard.so /usr/lib/\$BROWSER/plugins/
		fi
	done
fi

exit 0
EOF
chmod 755 $BUILDDIR/DEBIAN/postinst

echo Creating prerm script...
cat > $BUILDDIR/DEBIAN/prerm << EOF
#!/bin/bash

set -e

BROWSERS="mozilla firefox seamonkey"

for BROWSER in \$BROWSERS; do
	if [ -L /usr/lib/\$BROWSER/plugins/npDiveBoard.so ]; then
		rm -f /usr/lib/\$BROWSER/plugins/npDiveBoard.so
	fi
done

exit 0
EOF
chmod 755 $BUILDDIR/DEBIAN/prerm

echo Building Package...
fakeroot dpkg-deb --build $BUILDDIR $OUTDIR/$PKGNAME

if [ ! -f $OUTDIR/$PKGNAME ];then
	echo Something went wrong!
	exit 1
fi

