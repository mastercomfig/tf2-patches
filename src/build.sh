#!/usr/bin/env bash
set -e	# Stop on error
cd "$(dirname "$0")"

if pwd | grep -q " "; then
	echo "You have cloned the source directory into a path with spaces"
	echo "This will break a lot of thirdparty build scripts"
	echo "Please move the source directory somewhere without a space in the path"
	exit 1
fi

# Defaults
MAKE_SRT_FLAGS="NO_CHROOT=1 STEAM_RUNTIME_PATH="
MAKE_CFG="CFG=release"
MAKE_VERBOSE=""
VPC_GROUP="port"
CORES=$(nproc)
# shellcheck disable=SC2155
export CC="$(pwd)/devtools/bin/linux/ccache gcc"
# shellcheck disable=SC2155
export CXX="$(pwd)/devtools/bin/linux/ccache g++"
export VALVE_NO_AUTO_P4=1

while [[ ${1:0:1} == '-' ]]; do
	case "${1}" in
		"-v")
			MAKE_VERBOSE=1
		;;
		"-d")
			MAKE_CFG="CFG=debug"
		;;
		"-c")
			shift
			if [[ $1 == ?(-)+([[:digit:]]) ]]; then
				CORES=$1
			else
				echo "Not a number: ${1}"
				exit 1
			fi
		;;
		"-l")
			# shellcheck disable=SC2155
			export CC="$(pwd)/devtools/bin/linux/ccache clang"
			# shellcheck disable=SC2155
			export CXX="$(pwd)/devtools/bin/linux/ccache clang++"
			VPC_FLAGS+=" /define:CLANG"
		;;
		"-s")
			VPC_GROUP="dedicated"
			VPC_FLAGS+=" /define:DEDICATED"
		;;
		*)
			echo "Unknown flag ${1}"
			exit 1
		;;
	esac
	shift
done

build_thirdparty() {
  if [[ ! -f "thirdparty/$1/$2" ]]; then
    pushd .
    cd "thirdparty/$1/"
    local EXTRA_CFLAGS=$3
    local CFLAGS="-m32 -D_GLIBCXX_USE_CXX11_ABI=0 ${EXTRA_CFLAGS}"
    autoreconf -i 
    chmod u+x configure
    ./configure "CFLAGS=${CFLAGS}" \
      "CXXFLAGS=${CFLAGS}" \
      "LDFLAGS=-m32"
    make "-j$CORES"
    popd
  fi
}

build_thirdparty "protobuf-2.6.1" "src/.libs/libprotobuf.a"
build_thirdparty "libedit-3.1" "src/.libs/libedit.a" "-std=c99"
build_thirdparty "gperftools-2.0" ".libs/libtcmalloc_minimal.so" "-fpermissive -w"

pushd .

cd "./external/vpc/utils/vpc"
# shellcheck disable=SC2086
make "-j$CORES" CC="$CC" CXX="$CXX"

popd

# shellcheck disable=SC2086   # we want arguments to be split
devtools/bin/vpc_linux /define:WORKSHOP_IMPORT_DISABLE /define:SIXENSE_DISABLE /define:NO_X360_XDK \
				/define:RAD_TELEMETRY_DISABLED /nofpo /tf ${VPC_FLAGS} "+${VPC_GROUP}" /mksln games

mkdir -p "../game"
time make "${MAKE_SRT_FLAGS}" MAKE_VERBOSE="${MAKE_VERBOSE}" ${MAKE_CFG} \
		MAKE_JOBS="$CORES" -f games.mak "$@"
