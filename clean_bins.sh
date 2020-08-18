#!/bin/bash
# Run script within the directory
BINDIR=$(dirname "$(readlink -fn \"$0\")")
cd "${BINDIR}" || exit 2

set -e

dev_folder=../game

declare -a files=("default.exe" "hl2.exe" "srcds.exe" "bin/hammer.exe" "bin/hammer_dll.dll" "bin/engine.dll" "bin/GameUI.dll" "bin/replay.dll" "bin/launcher.dll" "bin/inputsystem.dll" "bin/MaterialSystem.dll" "bin/dedicated.dll" "bin/shaderapidx9.dll" "bin/vguimatsurface.dll" "bin/stdshader_dx9.dll" "bin/vgui2.dll" "bin/datacache.dll" "bin/sourcevr.dll" "bin/StudioRender.dll" "bin/SoundEmitterSystem.dll" "bin/bsppack.dll" "bin/FileSystem_Stdio.dll" "bin/scenefilecache.dll" "bin/vstdlib.dll" "bin/tier0.dll" "tf/bin/client.dll" "tf/bin/server.dll")
for F in "${files[@]}"; do
  rm "$dev_folder/$F" 2> /dev/null && echo "$dev_folder/$F"
  rm "$dev_folder/${F}_Debug_sentinel" 2> /dev/null && echo "$dev_folder/${F}_Debug_sentinel"
  rm "$dev_folder/${F}_Release_sentinel" 2> /dev/null && echo "$dev_folder/${F}_Release_sentinel"
done
