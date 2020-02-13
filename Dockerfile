FROM ubuntu:18.04

RUN mkdir -p /home/work
WORKDIR /home/work

RUN apt-get update --fix-missing
RUN apt-get -y install build-essential
RUN apt-get -y install nasm
RUN apt-get -y install gdb
RUN apt-get -y install xxd

CMD ["/bin/bash"]
