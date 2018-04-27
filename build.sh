#!/bin/bash

function die {
	echo "Libwallet build failed. Check the instructions to build it manually: https://github.com/exantech/monero-nodejs-libwallet"
	exit 1
}

CWD=`pwd`

git submodule update --init --recursive || die

pushd $CWD

if [ ! -d "monero/build" ]; then
	mkdir monero/build || die
fi

cd monero/build

ARCHIVE_DIR=`pwd`/lib

cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_GUI_DEPS=ON -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=$ARCHIVE_DIR .. || die
make easylogging wallet_merged epee lmdb unbound || die

popd

if [ ! -d "lib" ]; then
	mkdir lib || die
fi

cp $ARCHIVE_DIR/libeasylogging.a $ARCHIVE_DIR/libepee.a $ARCHIVE_DIR/liblmdb.a $ARCHIVE_DIR/libunbound.a $ARCHIVE_DIR/libwallet_merged.a lib/ || die