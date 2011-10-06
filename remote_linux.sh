

echo '
tmpbuild=$(mktemp -d /tmp/diveboard-db.XXXXXXXX)

echo $tmpbuild

cd $tmpbuild

git clone git@diveboard.plan.io:diveboard-db.git

cd "$tmpbuild"/diveboard-db/libdivecomputer
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

' | ssh "$1"



