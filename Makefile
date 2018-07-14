MONERO_BRANCH?="release-v0.12-multisig-wallet-assembled"
MONERO_BUILD_TYPE?=Release

BOOST_VERSION=1.66.0
BOOST_DIRNAME=boost_1_66_0

PWD=${shell pwd}
BOOST_LIBS=chrono,date_time,filesystem,program_options,regex,serialization,system,thread
THREADS?=${shell python -c 'import multiprocessing as mp; print(mp.cpu_count())'}

.PHONY: all
all: binding.gyp deps
	node_modules/.bin/node-pre-gyp configure build

.PHONY: clean
clean:
	rm -rf boost
	rm -rf monero/build
	rm -rf ${BOOST_DIRNAME}
	rm -rf deps
	rm -rf build
	rm -rf lib

${BOOST_DIRNAME}.tar.bz2: 
	curl -L -o "${BOOST_DIRNAME}.tar.bz2" \
            http://sourceforge.net/projects/boost/files/boost/${BOOST_VERSION}/${BOOST_DIRNAME}.tar.bz2/download

${BOOST_DIRNAME}: ${BOOST_DIRNAME}.tar.bz2
	tar xf ${BOOST_DIRNAME}.tar.bz2

.PHONY: boost
boost: ${BOOST_DIRNAME}
	cd ${BOOST_DIRNAME} && ./bootstrap.sh --with-libraries=${BOOST_LIBS}
	cd ${BOOST_DIRNAME} && ./b2 -j${THREADS} cxxflags=-fPIC cflags=-fPIC -a link=static \
		threading=multi threadapi=pthread --prefix=${PWD}/boost install

.PHONY: deps
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
		-DEMBEDDED_WALLET=1 \
		..

	cd monero/build && make -j${THREADS} wallet_merged epee easylogging lmdb unbound VERBOSE=1
	cp monero/build/lib/libwallet_merged.a ${PWD}/deps
