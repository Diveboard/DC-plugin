DIR=$(dirname $(readlink -f $0))

mkdir -p ~/rpmbuild/SOURCES
cp $DIR/build/packages/diveboard-1.1.0-alpha1*.tgz ~/rpmbuild/SOURCES/diveboard.tgz

rpmbuild --clean $DIR/diveboard-plugin.spec

rpmbuild -bp $DIR/diveboard-plugin.spec
rpmbuild -bc --short-circuit $DIR/diveboard-plugin.spec
rpmbuild -bi --short-circuit $DIR/diveboard-plugin.spec
rpmbuild -ba --short-circuit $DIR/diveboard-plugin.spec

cp ~/rpmbuild/RPMS/*/diveboard-plugin*.rpm $DIR/build/packages
