MONERO_BRANCH?="release-v0.14-exa-wallet-nodejs"
MONERO_BUILD_TYPE?=Release

BOOST_VERSION=1.66.0
BOOST_DIRNAME=boost_1_66_0

SODIUM_VERSION=1.0.17
SODIUM_DIRNAME=libsodium-${SODIUM_VERSION}
SODIUM_LIBRARY=${SODIUM_DIRNAME}/src/libsodium/.libs/libsodium.a

PWD=${shell pwd}
BOOST_LIBS=chrono,date_time,filesystem,program_options,regex,serialization,system,thread
THREADS?=1

.PHONY: all
all: binding.gyp deps
	node_modules/.bin/node-pre-gyp configure build

.PHONY: clean
clean:
	rm -rf boost
	rm -rf monero/build
	rm -rf ${BOOST_DIRNAME}
	rm -rf ${SODIUM_DIRNAME}
	rm -rf deps
	rm -rf build
	rm -rf lib

${BOOST_DIRNAME}.tar.bz2: 
	curl -L -o "${BOOST_DIRNAME}.tar.bz2" \
            http://sourceforge.net/projects/boost/files/boost/${BOOST_VERSION}/${BOOST_DIRNAME}.tar.bz2/download

${BOOST_DIRNAME}: ${BOOST_DIRNAME}.tar.bz2
	tar xf ${BOOST_DIRNAME}.tar.bz2
	cd ${BOOST_DIRNAME} && ./bootstrap.sh --with-libraries=${BOOST_LIBS}

boost: ${BOOST_DIRNAME}
	cd ${BOOST_DIRNAME} && ./b2 -j4 cxxflags=-fPIC cflags=-fPIC -a link=static \
		threading=multi threadapi=pthread --prefix=${PWD}/boost install

depsdir:
	mkdir -p deps/include

libsodium-${SODIUM_VERSION}.tar.gz:
	curl -L -o "libsodium-${SODIUM_VERSION}.tar.gz" https://github.com/jedisct1/libsodium/releases/download/${SODIUM_VERSION}/libsodium-${SODIUM_VERSION}.tar.gz

${SODIUM_LIBRARY}: libsodium-${SODIUM_VERSION}.tar.gz
	rm -rf ${SODIUM_DIRNAME}
	tar zxf libsodium-${SODIUM_VERSION}.tar.gz 
	cd ${SODIUM_DIRNAME} && \
	./configure --disable-shared && make -j${THREADS}

libsodium: depsdir ${SODIUM_LIBRARY}
	cp ${SODIUM_LIBRARY} deps
	cp -r ${SODIUM_DIRNAME}/src/libsodium/include/sodium deps/include
	cp -r ${SODIUM_DIRNAME}/src/libsodium/include/sodium.h deps/include

.PHONY: deps
deps: depsdir boost ${PWD}/deps/libwallet_merged.a
	cp boost/lib/*.a deps

monero:
	git clone --depth 1 --recurse-submodules -b ${MONERO_BRANCH} https://github.com/exantech/monero
	cp monero/src/wallet/api/wallet2_api.h include
	
${PWD}/deps/libwallet_merged.a: libsodium boost monero
	mkdir -p monero/build
	cd monero/build && cmake \
		-DARCH=default \
		-DBOOST_IGNORE_SYSTEM_PATHS=ON \
		-DBUILD_SHARED_LIBS=OFF -DBUILD_GUI_DEPS=ON \
		-DUSE_DEVICE_LEDGER=0 -DBUILD_TESTS=OFF -DSTATIC=ON \
		-DCMAKE_BUILD_TYPE=${MONERO_BUILD_TYPE} \
		-DBOOST_ROOT=${PWD}/boost \
		-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true \
		-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${PWD}/deps \
		-DEMBEDDED_WALLET=1 \
		-DSODIUM_INCLUDE_PATH=${PWD}/deps/include \
		-DSODIUM_LIBRARY=${PWD}/deps/libsodium.a \
		..

	cd monero/build && make -j${THREADS} wallet_merged epee easylogging lmdb unbound VERBOSE=1
	cp monero/build/lib/libwallet_merged.a ${PWD}/deps
