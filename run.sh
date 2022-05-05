#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"/game

MANGO="$(command -v mangohud || true)"
ARGS="-steam -game tf -insecure -novid -nojoy -nosteamcontroller -nohltv -particles 1 -noborder -particle_fallback 2 -dev -nobreakpad -console"
export LD_LIBRARY_PATH="$(pwd)/bin:$(pwd)/tf/bin"

if [[ "$1" == "-d" ]]; then
	# shellcheck disable=SC2086
	# shellcheck disable=SC2068
	lldb "$(pwd)/hl2_linux" -- -allowdebug ${ARGS} ${@:2}
elif [[ "$1" == "-b" ]]; then
	rm tf/cfg/config.cfg
	mkdir -p ../benchlogs
	# shellcheck disable=SC2086
	MANGOHUD_DLSYM=1 MANGOHUD_CONFIG="output_folder=$(pwd)/../benchlogs,toggle_logging=F12" mangohud \
		"$(pwd)"/hl2_linux ${ARGS} "+bind f12 demo_resume" "+fps_max 0" "+playdemo demos/${2}" "+demo_pause"
elif [[ "$1" == "-s" ]]; then
	if [[ "$2"  == "-d" ]]; then
		lldb "$(pwd)/srcds_linux" -- -allowdebug ${ARGS} ${@:3}
	else
		"$(pwd)/srcds_linux" ${ARGS} ${@:2}
	fi
else
	# shellcheck disable=SC2086
	# shellcheck disable=SC2068
	MANGOHUD_DLSYM=1 ${MANGO} "$(pwd)"/hl2_linux ${ARGS} $@
fi
