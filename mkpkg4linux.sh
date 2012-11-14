#!/bin/bash

tmpbuild=/tmp/diveboard_plugin.$(date '+%Y%m%d_%H%M%S_%N')
mkdir -p "$tmpbuild"

cd $tmpbuild
git clone --depth 1 git@diveboard.plan.io:diveboard-db.git

cd "$tmpbuild"/diveboard-db
git submodule update --recursive --init

cd "$tmpbuild"/diveboard-db/libdivecomputer
autoreconf --install
./configure
make

cd "$tmpbuild"/diveboard-db
./firebreath/prepcodeblocks.sh projects build

cd "$tmpbuild"/diveboard-db/build
make

"$tmpbuild"/diveboard-db/mktgz4linux.sh
"$tmpbuild"/diveboard-db/mkdeb4ubuntu.sh
"$tmpbuild"/diveboard-db/mkrpm4linux.sh

echo "Build done in $tmpbuild/diveboard-db/build/packages"
find "$tmpbuild/diveboard-db/build/packages"



if [[ -x "$HOME/Dropbox/Plugin Builds" ]]
then
  VERSION=`cat "$tmpbuild/diveboard-db/VERSION"`
  DROPDIR="$HOME/Dropbox/Plugin Builds/V$VERSION/linux_$(arch)_$(date '+%Y%m%d_%H%M%S_%N')"
  mkdir -p "$DROPDIR"
  cp -a "$tmpbuild/diveboard-db/build/packages/*" "$DROPDIR"
fi
