# using version 20.04 is important, a lower version like 18.04 causes a build issue in unit_tests
FROM ubuntu:20.04
LABEL Description="Build environment"
# ARG BOOST_VERSION=1.80.0
ARG DEBIAN_FRONTEND=noninteractive

ENV HOME /root

SHELL ["/bin/bash", "-c"]

# Granger Installs
RUN apt-get update && apt-get -y --no-install-recommends install \
  build-essential \
  clang \
  cmake \
  gdb \
  wget \
  libopenblas-dev \
  liblapack-dev \
  libarpack2-dev \
  libsuperlu-dev

# STEER installs:
RUN apt-get -y install \
  clang-tools \
  cppcheck \
  doxygen \
  g++ \
  gcc \
  git \
  libcunit1-dev \
  libxml2-utils \
  make \
  valgrind


RUN pip install matplotlib numpy pandas scipy statsmodels scikit-learn

# Granger Install
RUN cd ${HOME} && \
  wget --no-check-certificate --quiet \
    https://boostorg.jfrog.io/artifactory/main/release/1.77.0/source/boost_1_77_0.tar.gz && \
    tar xzf ./boost_1_77_0.tar.gz && \
    cd ./boost_1_77_0 && \
    ./bootstrap.sh && \
    ./b2 install && \
    cd .. && \
    rm -rf ./boost_1_77_0

# Granger Install
# Normally you would fetch the armadillo package from the official download link 
# but in this case the official download link was actually on a webpage and was not executing correctly
# Therefore, i just downloaded it locally, unzipped it. And then here I just copy the directy and install it
RUN cd ${HOME}
COPY armadillo-12.6.7 /armadillo-12.6.7
RUN cd armadillo-12.6.7 && \
  ./configure && \
  make && \
  make install
