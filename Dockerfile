FROM ubuntu:16.04

RUN apt-get update && apt-get -y install build-essential cmake pkg-config libboost-dev libpcsclite-dev \
   libssl-dev libzmq3-dev libunwind8-dev liblzma-dev git libsodium-dev libboost-all-dev \
   libreadline6-dev libldns-dev libexpat1-dev doxygen graphviz libgtest-dev curl python-software-properties \
   wget cl-chipz

ENV HOME=/tmp

RUN bash -c "curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.33.11/install.sh | bash"

RUN bash -c "source /tmp/.nvm/nvm.sh && nvm install v8 && nvm install v10 && nvm install v12"

RUN bash -c "chmod +x /tmp/.nvm/nvm.sh"

WORKDIR /tmp/monero-nodejs-libwallet
