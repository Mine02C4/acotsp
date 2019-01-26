FROM ubuntu:18.04
LABEL maintainer Naoya Niwa <naoya@am.ics.keio.ac.jp>
RUN apt-get update
RUN apt-get install -y build-essential libopenmpi-dev ssh
ENV PATH $PATH:/usr/lib64/openmpi/bin
ENV LD_LIBRARY_PATH $LD_LIBRARY_PATH:/usr/lib64/openmpi/lib
WORKDIR /usr/local/src/acotsp
COPY . .
RUN make -j$(nproc) all
ENTRYPOINT ["make", "run"]

