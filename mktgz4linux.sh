#!/bin/bash

VERSION=1.1.0-alpha1

DIR=$(dirname $(readlink -f $0))

BUILDDIR=$DIR/build/projects/DiveBoard/diveboard-plugin-$VERSION
OUTDIR=$DIR/build/packages

LIBDIVE=$DIR/libdivecomputer/src/.libs/libdivecomputer.so.0.0.0
LIBDIVEBOARD=$DIR/build/bin/DiveBoard/npDiveBoard.so

PKGNAME=diveboard-$VERSION.tgz

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

echo Creating install script...
cat > "$BUILDDIR/install.sh" << EOF
#!/bin/bash

set -e

SELF=\$(which "\$0")
BASEDIR=\$(dirname "\$SELF")

if [ ! -w /usr/lib ]; then
	echo "You need to be root to install the plugin"
	exit 1
fi

cp -a "\$BASEDIR/usr" /

BROWSERS="mozilla firefox seamonkey"

for BROWSER in \$BROWSERS; do
	if [ -d /usr/lib/\$BROWSER ];then
		mkdir -p /usr/lib/\$BROWSER/plugins
		ln -sf /usr/lib/diveboard/npDiveBoard.so /usr/lib/\$BROWSER/plugins/
	fi
done

exit 0
EOF
chmod 755 $BUILDDIR/install.sh

echo Creating uninstall script...
cat > $BUILDDIR/uninstall.sh << EOF
#!/bin/bash

set -e

if [ ! -d /usr/lib/diveboard ]; then
	echo "The plugin for Diveboard does not seem to be installed. Aborting uninstallation."
	exit 2
fi

if [ ! -w /usr/lib ]; then
	echo "You need to be root to uninstall the plugin"
	exit 1
fi

BROWSERS="mozilla firefox seamonkey"

for BROWSER in \$BROWSERS; do
	if [ -L /usr/lib/\$BROWSER/plugins/npDiveBoard.so ]; then
		rm -f /usr/lib/\$BROWSER/plugins/npDiveBoard.so
	fi
done

rm -fr /usr/lib/diveboard

exit 0
EOF
chmod 755 $BUILDDIR/uninstall.sh

echo Building Package...
cd $BUILDDIR/..
tar zcf $OUTDIR/$PKGNAME --owner=root --group=root $(basename "$BUILDDIR")

if [ ! -f $OUTDIR/$PKGNAME ];then
	echo Something went wrong!
	exit 1
fi

