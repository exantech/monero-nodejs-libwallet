FROM ubuntu:16.04

RUN apt-get update
RUN apt-get -y install build-essential cmake pkg-config libboost-dev libpcsclite-dev \
   libssl-dev libzmq3-dev libunwind8-dev liblzma-dev git libsodium-dev libboost-all-dev \
   libreadline6-dev libldns-dev libexpat1-dev doxygen graphviz libgtest-dev curl python-software-properties \
   wget cl-chipz

RUN bash -c "wget -O boost_1_58_0.tar.bz2 https://sourceforge.net/projects/boost/files/boost/1.58.0/boost_1_58_0.tar.bz2/download"

RUN bash -c "tar jxvf boost_1_58_0.tar.bz2"

WORKDIR boost_1_58_0

RUN bash -c "./bootstrap.sh"

RUN bash -c "./b2 cxxflags=-fPIC cflags=-fPIC -a link=static --with-chrono --with-date_time --with-filesystem --with-program_options --with-regex --with-serialization --with-system --with-thread --with-locale threading=multi threadapi=pthread stage"

RUN bash -c "cd /usr/src/gtest && cmake . && make && mv libg* /usr/lib/"

RUN bash -c "curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.33.11/install.sh | bash"

RUN bash -c "source /root/.nvm/nvm.sh && nvm install v8 && nvm install v10"

WORKDIR /usr/src

CMD bash -c "source /root/.nvm/nvm.sh && nvm use v8 && \
npm install && \
mkdir -p deps && \
find /boost_1_58_0 -name libboost*.a -exec cp {} deps \; && \
./node_modules/.bin/node-pre-gyp configure build"
