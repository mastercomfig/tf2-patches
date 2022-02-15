#!/usr/bin/env bash
set -e	# Stop on error
cd "$(dirname "$0")/src"

if pwd | grep -q " "; then
	echo "You have cloned the source directory into a path with spaces"
	echo "This will break a lot of thirdparty build scripts"
	echo "Please move the source directory somewhere without a space in the path"
	exit 1
fi

# Defaults
CF_SUPPRESSION="-w"
MAKE_SRT_FLAGS="NO_CHROOT=1 STEAM_RUNTIME_PATH="
MAKE_CFG="CFG=release"
MAKE_VERBOSE=""
VPC_FLAGS="/define:LTCG /define:CERT"
CORES=$(nproc)
# shellcheck disable=SC2155
export CC="$(pwd)/devtools/bin/linux/ccache gcc"
# shellcheck disable=SC2155
export CXX="$(pwd)/devtools/bin/linux/ccache g++"
export VALVE_NO_AUTO_P4=1

while [[ ${1:0:1} == '-' ]]; do
	case "${1}" in
		"-v")
			CF_SUPPRESSION=""
		;;
		"-vv")
			CF_SUPPRESSION=""
			MAKE_VERBOSE=1
		;;
		"-d")
			VPC_FLAGS="/no_ceg /nofpo"
		;;
		"-dd")
			VPC_FLAGS="/no_ceg /nofpo"
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
		"-r")
			MAKE_SRT_FLAGS="PATH=/bin:/usr/bin"
			CHROOT_NAME="$(pwd | sed 's/\//_/g')_"  # Trailing _ is required
			# shellcheck disable=SC2155
			export CC="$(pwd)/devtools/bin/linux/ccache gcc-9"
			# shellcheck disable=SC2155
			export CXX="$(pwd)/devtools/bin/linux/ccache g++-9"
		;;
		"-l")
			# shellcheck disable=SC2155
			export CC="$(pwd)/devtools/bin/linux/ccache clang"
			# shellcheck disable=SC2155
			export CXX="$(pwd)/devtools/bin/linux/ccache clang++"
			VPC_FLAGS+=" /define:CLANG"
		;;
		*)
			echo "Unknown flag ${1}"
			exit 1
		;;
	esac
	shift
done

if [[ -n ${CHROOT_NAME} ]]; then
	if [[ ! -f tools/runtime/linux/steamrt_scout_i386.tar.xz ]]; then
		wget https://repo.steampowered.com/steamrt-images-scout/snapshots/latest-container-runtime-depot/com.valvesoftware.SteamRuntime.Sdk-i386-scout-sysroot.tar.gz \
			-O tools/runtime/linux/steamrt_scout_i386.tar.xz --show-progress
	fi
	if ! schroot -l | grep -q "${CHROOT_NAME}"; then
		echo "schroot does not exist, creating..."
		sudo tools/runtime/linux/configure_runtime.sh "${CHROOT_NAME}" steamrt_scout_i386 linux32
	fi
fi

if [[ ! -f "thirdparty/protobuf-2.6.1/src/.libs/libprotobuf.a" ]]; then
	pushd .
	cd "thirdparty/protobuf-2.6.1/"
	if [[ -n ${CHROOT_NAME} ]]; then
		schroot --chroot "${CHROOT_NAME}" -- /bin/bash << EOF
			export PATH=/bin:/usr/bin
			export CC="$CC"
			export CXX="$CXX"
			autoreconf
			chmod u+x configure
			./configure "CFLAGS=-m32 -Wno-reserved-user-defined-literal -D_GLIBCXX_USE_CXX11_ABI=0 ${CF_SUPPRESSION}" \
				"CXXFLAGS=-m32 -Wno-reserved-user-defined-literal -D_GLIBCXX_USE_CXX11_ABI=0 ${CF_SUPPRESSION}" \
				"LDFLAGS=-m32"
			make "-j$CORES"
EOF
	else
		autoreconf
		chmod u+x configure
		./configure "CFLAGS=-m32 -Wno-reserved-user-defined-literal -D_GLIBCXX_USE_CXX11_ABI=0 ${CF_SUPPRESSION}" \
			"CXXFLAGS=-m32 -Wno-reserved-user-defined-literal -D_GLIBCXX_USE_CXX11_ABI=0 ${CF_SUPPRESSION}" \
			"LDFLAGS=-m32"
		make "-j$CORES"
	fi
	popd
fi

if [[ ! -f "./devtools/bin/vpc_linux" ]]; then
	pushd .
	cd "./external/vpc/utils/vpc"
	if [[ -n ${CHROOT_NAME} ]]; then
		schroot --chroot "${CHROOT_NAME}" -- /bin/bash << EOF
			export PATH=/bin:/usr/bin
			export CC="$CC"
			export CXX="$CXX"
			make "-j$CORES" CC="$CC" CXX="$CXX"
EOF
	else
		# shellcheck disable=SC2086
		make "-j$CORES" CC="$CC" CXX="$CXX"
	fi
	popd
fi

# shellcheck disable=SC2086   # we want arguments to be split
devtools/bin/vpc_linux /define:WORKSHOP_IMPORT_DISABLE /define:SIXENSE_DISABLE /define:NO_X360_XDK \
				/define:RAD_TELEMETRY_DISABLED /define:DISABLE_ETW /retail /tf ${VPC_FLAGS} +game /mksln games

time CFLAGS="${CF_SUPPRESSION}" CXXFLAGS="${CF_SUPPRESSION}" make ${MAKE_SRT_FLAGS} MAKE_VERBOSE="${MAKE_VERBOSE}" ${MAKE_CFG} \
		MAKE_JOBS="$CORES" -f games.mak "$@"
