FROM ubuntu:16.04

RUN apt-get update
RUN apt-get -y install build-essential cmake pkg-config libboost-dev libpcsclite-dev \
   libssl-dev libzmq3-dev libunwind8-dev liblzma-dev git libsodium-dev libboost-all-dev \
   libreadline6-dev libldns-dev libexpat1-dev doxygen graphviz libgtest-dev curl python-software-properties

RUN bash -c "cd /usr/src/gtest && cmake . && make && mv libg* /usr/lib/"

RUN bash -c "curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.33.11/install.sh | bash"

RUN bash -c "source /root/.nvm/nvm.sh && nvm install v8 && nvm install v10"

WORKDIR /usr/src

CMD bash -c "source /root/.nvm/nvm.sh && nvm use v8 && \
npm install && \
./node_modules/.bin/node-pre-gyp configure build && \
./node_modules/.bin/node-pre-gyp package publish --config /node-pre-gyp.config"

