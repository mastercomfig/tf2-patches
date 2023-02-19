#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"

ARGS="+sv_pure 1 +maxplayers 32 +sv_lan 1"

"$(pwd)"/srcds_linux -game tf -insecure -console -nomaster ${ARGS} $@
