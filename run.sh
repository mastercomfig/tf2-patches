#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"/../game
pwd
# shellcheck disable=SC2155  # it would be a very strange day indeed if pwd fails
export LD_LIBRARY_PATH="/usr/lib32:$(pwd)/bin:$(pwd)/tf/bin:${LD_LIBRARY_PATH}"
echo "LD_LIBRARY_PATH = ${LD_LIBRARY_PATH}"

MANGO="$(command -v mangohud || true)"
ARGS="-steam -game tf -insecure -novid -nojoy -nosteamcontroller -nohltv -particles 1 -noborder -particle_fallback 2 -dev"

if [[ "$1" == "-d" ]]; then
	# shellcheck disable=SC2086
	lldb ./hl2_linux -- -allowdebug ${ARGS}
else
	# shellcheck disable=SC2086
	MANGOHUD_DLSYM=1 ${MANGO} "$(pwd)"/hl2_linux ${ARGS}
fi
