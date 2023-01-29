# docker container prune
# docker build -t mmbasic-build:0.5 .
# docker run -it -v "C:\home-thwill\git_sandbox\github":/home/thwill/github mmbasic-build:0.5

FROM gcc:11.3.0

RUN apt-get update
RUN apt-get -y install cmake
RUN apt-get -y install gcc-arm-none-eabi
RUN apt-get -y install gcovr
RUN apt-get -y install gdb
# RUN apt-get -y install lcov
RUN apt-get -y install man
RUN apt-get -y install nano
RUN apt-get -y install ssh

RUN useradd -ms /bin/bash thwill

# MMB4L unit-tests test for '/usr/bin/cp' existing.
RUN ln -s /bin/cp /usr/bin/cp

USER thwill
WORKDIR /home/thwill

RUN ln -s ~/host-home-thwill/.ssh ~/.ssh
RUN ln -s ~/host-home-thwill/.terminfo ~/.terminfo
RUN ln -s ~/host-home-thwill/git_sandbox/github ~/github

RUN mkdir local-mmb4l
RUN ln -s ~/github/mmb4l/examples local-mmb4l/examples
RUN ln -s ~/github/mmb4l/resources local-mmb4l/resources
RUN ln -s ~/github/mmb4l/sptools local-mmb4l/sptools
RUN ln -s ~/github/mmb4l/tests local-mmb4l/tests
RUN ln -s ~/github/mmb4l/src local-mmb4l/src
RUN ln -s ~/github/mmb4l/tools local-mmb4l/tools
RUN ln -s ~/github/mmb4l/build-all.sh local-mmb4l/build-all.sh
RUN ln -s ~/github/mmb4l/ChangeLog local-mmb4l/ChangeLog
RUN ln -s ~/github/mmb4l/CMakeLists.txt local-mmb4l/CMakeLists.txt
RUN ln -s ~/github/mmb4l/LICENSE local-mmb4l/LICENSE
RUN ln -s ~/github/mmb4l/LICENSE.MIT local-mmb4l/LICENSE.MIT
RUN ln -s ~/github/mmb4l/LICENSE.MMBasic local-mmb4l/LICENSE.MMBasic
RUN ln -s ~/github/mmb4l/package.sh local-mmb4l/package.sh
RUN ln -s ~/github/mmb4l/README.md local-mmb4l/README.md
RUN ln -s ~/github/mmb4l/TODO local-mmb4l/TODO

RUN mkdir ~/.mmbasic
RUN ln -s ~/github/mmb4l/resources/mmbasic.nanorc ~/.mmbasic/mmbasic.nanorc
RUN ln -s ~/github/mmb4l/resources/mmbasic.syntax.nanorc ~/.mmbasic/mmbasic.syntax.nanorc

RUN echo "" >> /home/thwill/.bashrc
RUN echo "LS_COLORS=$LS_COLORS:'ow=1;31:' ; export LS_COLORS" >> /home/thwill/.bashrc
#RUN echo "alias generate-coverage='lcov -c -d . -o main_coverage.info && genhtml main_coverage.info --output-directory ~/host-home-thwill/tmp/mmb4l-coverage'" >> /home/thwill/.bashrc
RUN echo "alias gcovr-mmb4l='mkdir -p ~/host-home-thwill/tmp/mmb4l-coverage && gcovr -r .. --html-details ~/host-home-thwill/tmp/mmb4l-coverage/coverage.html'" >> /home/thwill/.bashrc

#USER root
#EXPOSE 22
#ENTRYPOINT service ssh restart && bash

#WORKDIR /app
#
#COPY mmb4l-2022.01.00-a4-x86_64 mmb4l-0.4
#
#CMD ["sleep", "60"]
#CMD ["/app/mmb4l-0.4/mmbasic"]
