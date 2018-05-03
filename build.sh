#!/bin/bash

function die {
	echo "Libwallet build failed. Check the instructions to build it manually: https://github.com/exantech/monero-nodejs-libwallet"
	exit 1
}

MULTISIG_BRANCH=release-v0.12-multisig-wallet-assembled
CWD=`pwd`

pushd $CWD

if [ ! -d "monero" ]; then
	git clone -b $MULTISIG_BRANCH https://github.com/exantech/monero || die
fi

cd monero
git submodule update --init --recursive || die

if [ ! -d "build" ]; then
	mkdir build || die
fi

cd build

ARCHIVE_DIR=`pwd`/lib

cmake -DBUILD_SHARED_LIBS=OFF -DBUILD_GUI_DEPS=ON -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=$ARCHIVE_DIR .. || die
make easylogging wallet_merged epee lmdb unbound || die

popd

if [ ! -d "lib" ]; then
	mkdir lib || die
fi

cp $ARCHIVE_DIR/libeasylogging.a $ARCHIVE_DIR/libepee.a $ARCHIVE_DIR/liblmdb.a $ARCHIVE_DIR/libunbound.a $ARCHIVE_DIR/libwallet_merged.a lib/ || die
