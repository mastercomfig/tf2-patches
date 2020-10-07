#!/bin/bash
#
# Run script within the directory
BIN_DIR=$(dirname "$(readlink -fn "$0")")
cd "${BIN_DIR}" || exit 2

set -e

DEV_DIR=../../game
CLEAN_DIR=${DEV_DIR}_clean
rm -rf ${CLEAN_DIR}
mkdir -p ${CLEAN_DIR}/{bin,tf/{bin,custom}}
cp -rf copy/* ${CLEAN_DIR}

declare -a FILES=(
                  {hl2,srcds}.exe
                  bin/engine.dll
                  bin/GameUI.dll
                  bin/replay.dll
                  bin/launcher.dll
                  bin/inputsystem.dll
                  bin/{Material,SoundEmitter}System.dll
                  bin/dedicated.dll
                  bin/ServerBrowser.dll
                  bin/{shaderapi,stdshader_}dx9.dll
                  bin/vgui{matsurface,2}.dll
                  bin/{data,scenefile}cache.dll
                  bin/sourcevr.dll
                  bin/StudioRender.dll
                  bin/bsppack.dll
                  bin/FileSystem_Stdio.dll
                  bin/vstdlib.dll
                  bin/tier0.dll
                  tf/bin/{client,server}.dll
                 )
for F in "${FILES[@]}"; do
  cp -f ${DEV_DIR}/${F} ${CLEAN_DIR}/${F}
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
