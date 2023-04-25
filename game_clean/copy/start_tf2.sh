#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

ARGS="-steam -game tf -insecure -novid -nojoy -nosteamcontroller -nobreakpad"

$HOME/.steam/bin32/steam-runtime/run.sh ${LAUNCH_PREFIX} "$(pwd)"/hl2.sh ${ARGS} $@
