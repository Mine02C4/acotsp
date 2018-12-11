FROM centos:7
LABEL maintainer Naoya Niwa <naoya@am.ics.keio.ac.jp>
RUN yum update -y
RUN yum install -y gcc gcc-c++ glibc make openmpi-devel
ENV PATH $PATH:/usr/lib64/openmpi/bin
ENV LD_LIBRARY_PATH $LD_LIBRARY_PATH:/usr/lib64/openmpi/lib
WORKDIR /usr/local/src/acotsp
COPY . .
RUN make -j$(nproc) all
ENTRYPOINT ["make", "run"]
#ENTRYPOINT ["bash"]
