#!/bin/bash

sudo docker run -it -v /home/thwill/github/thwill1000:/mnt/thwill mmb4l-build:0.6 bash -c "cd /mnt/thwill/mmb4l;./docker/mmb4l-build-0.6/build-gcc-7.4.sh"
echo "foo"
sudo chown thwill:thwill build-release-gcc-7.4/mmb4l-0.6.0-x86_64-glibc-2.17.tgz
echo "bar"
cp build-release-gcc-7.4/mmb4l-0.6.0-x86_64-glibc-2.17.tgz .
echo "wombat"
