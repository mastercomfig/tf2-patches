#!/usr/bin/env bash
#
# TODO: Properly handle dedicated server builds
# Run script within the directory
BIN_DIR=$(dirname "$(readlink -fn "$0")")
cd "${BIN_DIR}" || exit 2

set -e

DEV_DIR=../game
CLEAN_DIR=../game_dist
CLEAN_DEBUG_DIR=${CLEAN_DIR}_debug
rm -rf ${CLEAN_DIR}
rm -rf ${CLEAN_DEBUG_DIR}
mkdir -p ${CLEAN_DIR}/{bin,tf/bin}
mkdir -p ${CLEAN_DEBUG_DIR}/{bin,tf/bin}
cp -rf copy/* ${CLEAN_DIR}

DLL_EXT=.so
DBG_EXT=.dbg
MAP_EXT=.map

declare -a EXECUTABLES=(
                  {hl2,srcds}_linux
                  bin/vtf2tga_linux32
                  bin/vpk_linux32
                 )

declare -a DLLS=(
                  bin/bsppack
                  bin/bugreporter_public
                  bin/datacache
                  bin/engine
                  bin/filesystem_stdio
                  bin/GameUI
                  bin/inputsystem
                  bin/launcher
                  bin/libtier0
                  bin/libtogl
                  bin/libvstdlib
                  bin/materialsystem
                  bin/replay
                  bin/scenefilecache
                  bin/ServerBrowser
                  bin/shaderapidx9
                  bin/shaderapiempty
                  bin/soundemittersystem
                  bin/sourcevr
                  bin/stdshader_dx9
                  bin/studiorender
                  bin/vgui2
                  bin/vguimatsurface
                  bin/vtex_dll
                  tf/bin/client
                  tf/bin/server
                 )

for F in "${EXECUTABLES[@]}"; do
  # This will fail at least once, since either hl2 or srcds exists, but not both.
  cp -f ${DEV_DIR}/${F} ${CLEAN_DIR}/${F} || true
  cp -f ${DEV_DIR}/${F}$MAP_EXT ${CLEAN_DEBUG_DIR}/${F}$MAP_EXT || true
  cp -f ${DEV_DIR}/${F}$DBG_EXT ${CLEAN_DEBUG_DIR}/${F}$DBG_EXT || true
done

for F in "${DLLS[@]}"; do
  cp -f ${DEV_DIR}/${F}$DLL_EXT ${CLEAN_DIR}/${F}$DLL_EXT
  cp -f ${DEV_DIR}/${F}$DLL_EXT$DBG_EXT ${CLEAN_DEBUG_DIR}/${F}$DLL_EXT$DBG_EXT
done

declare -a FILES_REPO=(
                  ../LICENSE_SDK
                  ../.github/README.md
                  ../thirdpartylegalnotices.txt
                  ./link.sh
                 )

for F in "${FILES_REPO[@]}"; do
  ORIG=$(basename ${F})
  cp -f ${F} ${CLEAN_DIR}/${ORIG}
done
