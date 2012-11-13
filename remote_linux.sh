#!/bin/bash

scp mkpkg4linux.sh $1:/tmp/
ssh $1 "bash -c 'screen -S compil -d -m /tmp/mkpkg4linux.sh'"
