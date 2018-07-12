MONERO_BRANCH?="release-v0.12-multisig-wallet-assembled"
MONERO_BUILD_TYPE=release

BOOST_VERSION=1.66.0
BOOST_VERSION2=1_66_0


PWD=${shell pwd}
BOOST_LIBS=chrono,date_time,filesystem,program_options,regex,serialization,system,thread
THREADS=${shell sysctl -n hw.ncpu}

.PHONY: all
all: binding.gyp deps
	node-pre-gyp configure build
clean:
	rm -rf boost
	rm -rf monero/build
	rm -rf boost_${BOOST_VERSION2}
	rm -rf deps
	rm -rf build

boost_${BOOST_VERSION2}.tar.bz2: 
	curl -L -o "boost_${BOOST_VERSION2}.tar.bz2" \
            http://sourceforge.net/projects/boost/files/boost/${BOOST_VERSION}/boost_${BOOST_VERSION2}.tar.bz2/download

boost_${BOOST_VERSION2}: boost_${BOOST_VERSION2}.tar.bz2
	tar xf boost_${BOOST_VERSION2}.tar.bz2

boost: boost_${BOOST_VERSION2}
	cd boost_${BOOST_VERSION2} && ./bootstrap.sh --with-libraries=${BOOST_LIBS}
	cd boost_${BOOST_VERSION2} && ./b2 -j${THREADS} cxxflags=-fPIC cflags=-fPIC -a link=static \
		threading=multi threadapi=pthread --prefix=${PWD}/boost install

deps: boost monero/build
	mkdir -p deps
	cp boost/lib/*.a deps

monero:
	git clone --depth 1 --recurse-submodules -b ${MONERO_BRANCH} https://github.com/exantech/monero
	cp monero/src/wallet/api/wallet2_api.h include
	
monero/build: boost monero
	mkdir -p monero/build
	mkdir -p deps
	cd monero/build && cmake -DBOOST_IGNORE_SYSTEM_PATHS=ON -DBUILD_SHARED_LIBS=OFF -DBUILD_GUI_DEPS=ON \
		-DUSE_DEVICE_LEDGER=0 -DBUILD_TESTS=OFF -DSTATIC=ON \
		-DCMAKE_BUILD_TYPE=${MONERO_BUILD_TYPE} \
		-DBOOST_ROOT=${PWD}/boost \
		-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=true \
		-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${PWD}/deps \
		-DZMQ_INCLUDE_PATH=${PWD}/zmq \
		..

#		-D OPENSSL_ROOT_DIR=${OPENSSL} \
#		-D OPENSSL_CRYPTO_LIBRARY=${OPENSSL}/libcrypto.a -D OPENSSL_SSL_LIBRARY=${OPENSSL}/libssl.a \

	cd monero/build && make -j${THREADS} wallet_merged epee easylogging lmdb unbound VERBOSE=1
	cp monero/build/lib/libwallet_merged.a ${PWD}/deps
