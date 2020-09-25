#!/usr/bin/env sh
set -e	# Stop on error
cd "$(dirname "$0")"

if pwd | grep -q " "; then
	echo "You have cloned the source directory into a path with spaces"
	echo "This will break a lot of thirdparty build scripts"
	echo "Please move the source directory somewhere without a space in the path"
	exit 1
fi

: "${CORES:=$(nproc)}"

if [ ! -f ./thirdparty/gperftools-2.0/.libs/libtcmalloc_minimal.so ]; then
	cd ./thirdparty/gperftools-2.0
	aclocal
	automake --add-missing
	autoconf
	chmod u+x configure
	./configure --enable-frame-pointers "CFLAGS=-m32 -w" "CXXFLAGS=-m32 -w" "LDFLAGS=-m32"
	make "-j$CORES"
	cd ../..
fi

if [ ! -f ./thirdparty/protobuf-2.5.0/src/.libs/libprotobuf.a ]; then
	cd ./thirdparty/protobuf-2.5.0
	aclocal
	automake --add-missing
	autoconf
	chmod u+x configure
	./configure "CFLAGS=-m32 -w -D_GLIBCXX_USE_CXX11_ABI=0" "CXXFLAGS=-m32 -w -D_GLIBCXX_USE_CXX11_ABI=0" "LDFLAGS=-m32" --enable-shared=no
	make "-j$CORES"
cd ../..
fi

if [ ! -f ./thirdparty/libedit-3.1/src/.libs/libedit.so ]; then
	cd ./thirdparty/libedit-3.1
	aclocal
	automake --add-missing
	autoconf
	chmod u+x ./configure
	./configure "CFLAGS=-m32 -w" "CXXFLAGS=-m32 -w" "LDFLAGS=-m32"
	make "-j$CORES"
	cd ../..
fi

if [ ! -f ./devtools/bin/vpc_linux ]; then
	cd ./external/vpc/utils/vpc
	make "-j$CORES"
	cd ../../../..
fi

case "$1" in
	-dd)
		echo "-------------------------------------------------"
		echo "                    WARNING"
		echo "    You are probably about to waste your time"
		echo "        Debug builds currently don't work"
		echo "    You'll probably get an error about vphysics"
		echo "-------------------------------------------------"
		sleep 2

		./creategameprojects_debug.sh
		;;
	-d)
		./creategameprojects_dev.sh
		;;
	*)
		./creategameprojects.sh
		;;
esac

if echo "$*" | grep -q -- "-c"; then
	MAKEARGS="clean"
else
	MAKEARGS="all"
fi

export VALVE_NO_AUTO_P4=1

case "$*" in
	-vv)
		make NO_CHROOT=1 STEAM_RUNTIME_PATH='' MAKE_JOBS=1 MAKE_VERBOSE=1 -f games.mak "$MAKEARGS"
		;;
	-v)
		make NO_CHROOT=1 STEAM_RUNTIME_PATH='' MAKE_JOBS=1 -f games.mak "$MAKEARGS"
		;;
	*)
		CFLAGS="-w" CXXFLAGS="-w" make NO_CHROOT=1 STEAM_RUNTIME_PATH='' MAKE_JOBS="$CORES" -f games.mak "$MAKEARGS"
		;;
esac
