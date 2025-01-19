# Install docker.
sudo snap install docker

# Add 'docker' group.
sudo groupadd docker

# Add current user to 'docker' group.
sudo usermod -aG docker $USER

docker exec -it ubuntu:18.04 bash

# List running containers.
docker ps -a

# Remove all stopper containers.
docker container prune

docker container run -it -v /home/thwill:/home/thwill ubuntu:18.04 /bin/bash

# List available images.
docker image list

# Remove image.
docker remove <image>

# Not sure, but needed it for debian11.
docker run --rm --privileged multiarch/qemu-user-static --reset -p yes -c yes

# Enable multi-platform builds.
docker buildx create --use --platform=linux/arm/v6,linux/arm64,linux/amd64 --name multi-platform-builder
docker buildx inspect --bootstrap
docker buildx build -t mmb4l-build:0.7 --platform linux/arm/v6,linux/arm64,linux/amd64 .

# Build images.
docker buildx build --load -t mmb4l-build-arm64:0.7 --platform linux/arm64 .
docker buildx build --load -t mmb4l-build-amd64:0.7 --platform linux/amd64 .
docker buildx build --load -t mmb4l-build-armv6l:0.7 --platform linux/arm/v6 .

# Run containers.
docker run -it -e DISPLAY=$DISPLAY --platform linux/amd64 --net=host -v /home/thwill:/home/thwill mmb4l-build-amd64:0.7 /bin/bash
docker run -it -e DISPLAY=$DISPLAY --platform linux/arm64 --net=host -v /home/thwill:/home/thwill mmb4l-build-arm64:0.7 /bin/bash
docker run -it -e DISPLAY=$DISPLAY -e QEMU_CPU=arm1176 --platform linux/arm/v6 --net=host -v /home/thwill:/home/thwill mmb4l-build-armv6l:0.7 /bin/bash

# Build a RPi-Zero docker image from SD card

Adapted from https://ricardodeazambuja.com/rpi/2020/12/29/rpi2docker/

 1. Connect SD card from RPi-Zero to Ubuntu desktop.

 2. Make a tarball from its contents:
 ```
 sudo tar -cvf raspbian-pizero.tar --exclude="dev/*" --exclude="proc/*" --exclude="tmp/*" --exclude="run/*" --exclude="mnt/*" --exclude="media/*" --exclude="lost+found" -C /media/$USER/rootfs/ .
 ```

 3. I think you need this:
```
docker run --rm --privileged multiarch/qemu-user-static --reset -p yes -c yes
```
  or possibly just this:
```
docker run --privileged linuxkit/binfmt:v0.8
```

 4. Import file into new image:
 ```
 docker import raspbian-pizero.tar thwill/raspbian-pizero-base
 ```

 5. Create a Dockerfile with this contents:
```
FROM thwill/raspbian-pizero-base

RUN apt-get -y install libsdl2-image-dev

USER thwill
WORKDIR /home/thwill
```

 6. Build a new image using this Dockerfile:
```
docker build -t thwill-sdl2/linux/arm/v6 -f Dockerfile-raspbian-pizero .
```

 7. Run the image:
```
docker run -it --rm -e DISPLAY=$DISPLAY -e QEMU_CPU=arm1176 --net=host -v /home/thwill:/home/thwill thwill-sdl2/linux/arm/v6 /bin/bash
```

