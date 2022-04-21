#!/bin/sh

ROOT=$(pwd)

# download systemC
wget https://www.accellera.org/images/downloads/standards/systemc/systemc-2.3.3.tar.gz \
  && tar xzf systemc-2.3.3.tar.gz && rm systemc-2.3.3.tar.gz
# compile systemC
mkdir -p systemc-2.3.3/build && cd systemc-2.3.3/build
cmake .. -DCMAKE_CXX_STANDARD=17
make

cd $ROOT

wget https://sourceforge.net/projects/nanapro/files/latest/download -O nana.zip \
  && unzip nana.zip && rm nana.zip
mkdir -p nana/built && cd nana/built
cmake .. && make
