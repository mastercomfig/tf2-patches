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

declare -a EXES=(
  {hl2,srcds}
  bin/shadercompile
  bin/splitskybox
  bin/hammer
  bin/vvis
  bin/vrad
  bin/vbsp
  bin/bspzip
  bin/vpk
  bin/modelbrowser
  bin/dmxedit
  bin/dmxconvert
  bin/elementviewer
  bin/captioncompiler
  bin/glview
  bin/motionmapper
  bin/scenemanager
  bin/MakeScenesImage
  bin/qc_eyes
  bin/hlmv
  bin/studiomdl
  bin/vtex
  bin/pfm2tgas
  bin/vtf2tga
  bin/dist2alpha
  bin/height2ssbump
  bin/height2normal
  bin/normal2ssbump
  bin/mksheet
  bin/inputtest
  bin/matsys_regressiontest
  bin/materialsystemtest
  bin/panel_zoo
  bin/newdat
  bin/simdtest
  bin/shaderapitest
  bin/unittest
)

declare -a DLLS_CI=(
  bin/engine
  bin/replay
  bin/launcher
  bin/inputsystem
  bin/{Material,SoundEmitter}System
  bin/soundsystem
  bin/dedicated
  bin/shaderapi{dx9,empty}
  bin/stdshader_{dx6,dx7,dx8,dx9,dbg}
  bin/vgui{matsurface,2}
  bin/{data,scenefile}cache
  bin/sourcevr
  bin/StudioRender
  bin/bsppack
  bin/FileSystem_Stdio
  bin/vstdlib
  bin/tier0
  bin/unicode
  bin/vvis_dll
  bin/vrad_dll
  bin/hammer_dll
  bin/shadercompile_dll
  bin/texturecompile_dll
  bin/AdminServer
  bin/mdllib
  bin/tools/commedit
  bin/tools/pet
  bin/tools/vmt
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
