#!/bin/bash
#
# Run script within the directory
BIN_DIR=$(dirname "$(readlink -fn "$0")")
cd "${BIN_DIR}" || exit 2

set -e

DEV_DIR=../game
CLEAN_DIR=../${DEV_DIR}_clean
CLEAN_DEBUG_DIR=${CLEAN_DIR}_debug
rm -rf ${CLEAN_DIR}
rm -rf ${CLEAN_DEBUG_DIR}
mkdir -p ${CLEAN_DIR}/{bin,tf/bin}
mkdir -p ${CLEAN_DEBUG_DIR}/{bin,tf/bin}
cp -rf copy/* ${CLEAN_DIR}
cp -rf copy/* ${DEV_DIR}

declare -a FILES=(
                  {hl2,srcds}.exe
                 )

declare -a DLLS_CI=(
                  bin/engine
                  bin/replay
                  bin/launcher
                  bin/inputsystem
                  bin/{Material,SoundEmitter}System
                  bin/dedicated
                  bin/{shaderapi,stdshader_}dx9
                  bin/vgui{matsurface,2}
                  bin/{data,scenefile}cache
                  bin/sourcevr
                  bin/StudioRender
                  bin/bsppack
                  bin/FileSystem_Stdio
                  bin/vstdlib
                  bin/tier0
                  tf/bin/{client,server}
                 )

declare -a DLLS=(
                  bin/GameUI
                  bin/ServerBrowser
                 )

for F in "${FILES[@]}"; do
  cp -f ${DEV_DIR}/${F} ${CLEAN_DIR}/${F}
done

for F in "${DLLS_CI[@]}"; do
  cp -f ${DEV_DIR}/${F}.dll ${CLEAN_DIR}/${F}.dll
  cp -f ${DEV_DIR}/${F,,}.pdb ${CLEAN_DEBUG_DIR}/${F,,}.pdb
done

for F in "${DLLS[@]}"; do
  cp -f ${DEV_DIR}/${F}.dll ${CLEAN_DIR}/${F}.dll
  cp -f ${DEV_DIR}/${F}.pdb ${CLEAN_DEBUG_DIR}/${F}.pdb
done

declare -a FILES=(
                  ../LICENSE_SDK
                  ../.github/README.md
                  ../thirdpartylegalnotices.txt
                 )
for F in "${FILES[@]}"; do
  ORIG=$(basename ${F})
  cp -f ${F} ${CLEAN_DIR}/${ORIG}
done
