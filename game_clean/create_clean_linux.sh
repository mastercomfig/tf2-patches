#!/bin/bash
#
# Run script within the directory
BIN_DIR=$(dirname "$(readlink -fn "$0")")
cd "${BIN_DIR}" || exit 2

set -e

DEV_DIR=../game
CLEAN_DIR=../game_dist
CLEAN_DEBUG_DIR=${CLEAN_DIR}_debug
rm -rf ${CLEAN_DIR}
rm -rf ${CLEAN_DEBUG_DIR}
mkdir -p ${CLEAN_DIR}/{bin,tf/bin,bin/tools}
mkdir -p ${CLEAN_DEBUG_DIR}/{bin,tf/bin,bin/tools}
cp -rf copy/* ${CLEAN_DIR}
cp -rf copy/* ${DEV_DIR}

EXE_EXT=
DLL_EXT=.so
PDB_EXT=.dbg

declare -a FILES=(
                  {hl2_linux,hl2.sh}
                 )

declare -a DLLS_CI=(
                  bin/engine
                  bin/replay
                  bin/launcher
                  bin/inputsystem
                  bin/{material,soundemitter}system
                  bin/{shaderapi,stdshader_}dx9
                  bin/vgui{matsurface,2}
                  bin/{data,scenefile}cache
                  bin/sourcevr
                  bin/studiorender
                  bin/bsppack
                  bin/filesystem_stdio
                  bin/libvstdlib
                  bin/libtier0
                  tf/bin/{client,server}
                 )

declare -a DLLS=(
  bin/GameUI
  bin/ServerBrowser
  bin/FileSystemOpenDialog
)

for F in "${EXES[@]}"; do
  cp -f ${DEV_DIR}/${F}.exe ${CLEAN_DIR}/${F}.exe
done

for F in "${DLLS_CI[@]}"; do
  cp -f ${DEV_DIR}/${F}$DLL_EXT ${CLEAN_DIR}/${F}$DLL_EXT
#  cp -f ${DEV_DIR}/${F,,}.pdb ${CLEAN_DEBUG_DIR}/${F,,}.pdb
done

for F in "${DLLS[@]}"; do
  cp -f ${DEV_DIR}/${F}$DLL_EXT ${CLEAN_DIR}/${F}$DLL_EXT
#  cp -f ${DEV_DIR}/${F}.pdb ${CLEAN_DEBUG_DIR}/${F}.pdb
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
