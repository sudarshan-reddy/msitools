FROM debian:buster

RUN apt-get -y update
RUN apt-get -y install bison git python3 python3-pip python3-setuptools \
                       python3-wheel ninja-build valac libgsf-1-dev libgcab-dev \
                       libgirepository1.0-dev
RUN pip3 install meson
RUN git clone https://github.com/sudarshan-reddy/msitools

WORKDIR msitools
RUN git submodule update --init --recursive
RUN meson setup builddir
WORKDIR builddir
RUN meson compile
