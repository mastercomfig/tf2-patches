#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"/game

export LD_LIBRARY_PATH='./bin:./tf/bin'

ARGS=('-steam' '-game tf' '-insecure' '-novid' '-nojoy' '-nosteamcontroller' '-nohltv' '-particles 1' '-noborder' '-particle_fallback 2' '-dev' '-nobreakpad' '-console')

while [[ ${#1} == 2 && "${1:0:1}" == '-' ]]; do
    case "${1}" in
    -s)
    	PREPEND=()
    	: "${EXE:="./srcds_linux"}"
	;;
 	-b)
 		if [[ -n "${!PREPEND*}" ]]; then
			echo "conflicting flags given"
			exit 1
		fi
 		[[ ${MANGOHUD:="$(command -v mangohud)"} ]] || (echo mangohud not found && exit 1)
 		PREPEND=('MANGOHUD_DLSYM=1' "MANGOHUD_CONFIG='output_folder=$(pwd)/../benchlogs,toggle_logging=F12'" "${MANGOHUD}")
 		shift
 		if [[ -z "${1}" ]]; then
 			echo "no demo provided"
 			exit 1
		fi
 		ARGS+=('+bind f12 demo_resume' '+fps_max 0' "+playdemo demos/${1}" '+demo_pause')
	;;
	-d)
		if (( ${#PREPEND[@]} )); then
			echo "conflicting flags given"
			exit 1
		fi
		[[ ${LLDB:="$(command -v lldb)"} ]] || (echo "lldb not found" && exit 1)
		PREPEND=("${LLDB}")
		ARGS=("--" "${ARGS[@]}" '-allowdebug')
	;;
	-g)
		if (( ${#PREPEND[@]} )); then
			echo "conflicting flags given"
			exit 1
		fi
		[[ ${GDB:="$(command -v gdb)"} ]] || (echo "gdb not found" && exit 1)
		PREPEND=("${GDB}" "--args")
		ARGS+=("-allowdebug")
	;;
	*)
		echo "Unknown flag ${1}"
		echo "Usage: ${0} [-b <demo name>] [-dgs] [game arguments]"
		exit 1
	;;
	esac
	shift
done

if [[ -z "${!PREPEND*}" ]]; then
	PREPEND=("$(command -v mangohud || command -v env)")
fi
: "${EXE:="./hl2_linux"}"


# shellcheck disable=SC2068
"${PREPEND[@]}" "${EXE}" ${ARGS[@]} "${@}"
