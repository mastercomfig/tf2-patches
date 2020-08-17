#!/bin/bash
# Run script within the directory
BINDIR=$(dirname "$(readlink -fn "$0")")
cd "${BINDIR}" || exit 2

set -e

clean_folder=../../game_clean
dev_folder=../../game
mkdir -p $clean_folder
mkdir -p $clean_folder/bin
mkdir -p $clean_folder/tf/bin
cp -f *.bat $clean_folder

declare -a files=("steam_appid.txt" "hl2.exe" "srcds.exe" "bin/engine.dll" "bin/GameUI.dll" "bin/replay.dll" "bin/launcher.dll" "bin/inputsystem.dll" "bin/MaterialSystem.dll" "bin/dedicated.dll" "bin/shaderapidx9.dll" "bin/vguimatsurface.dll" "bin/stdshader_dx9.dll" "bin/vgui2.dll" "bin/datacache.dll" "bin/sourcevr.dll" "bin/StudioRender.dll" "bin/SoundEmitterSystem.dll" "bin/bsppack.dll" "bin/FileSystem_Stdio.dll" "bin/scenefilecache.dll" "bin/vstdlib.dll" "bin/tier0.dll" "tf/bin/client.dll" "tf/bin/server.dll")
for F in "${files[@]}"; do
  cp -f $dev_folder/$F $clean_folder/$F
done
