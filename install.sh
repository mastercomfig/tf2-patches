#!/usr/bin/env bash

echo Assuming TF2 resources exist at ../game
echo Resolved install location to "$(realpath ../game)"

echo

echo Build products are copied by build.sh
echo If you overwrote them you have to rebuild or manually copy them over for now.

echo

# TODO(maximsmol): copy build products manually here since make
# will skip this step if no sources changed

# declare -A map

# map[bsppack.so]=''
# map[datacache.so]=''
# map[dedicated.so]=''
# map[engine.so]=''
# map[filesystem_stdio.so]=''
# map[GameUI.so]=''
# map[inputsystem.so]=''
# map[launcher.so]=''
# map[libtier0.so]=''
# map[libtogl.so]=''
# map[libvstdlib.so]=''
# map[materialsystem.so]=''
# map[replay.so]=''
# map[scenefilecache.so]=''
# map[ServerBrowser.so]=''
# map[shaderapidx9.so]=''
# map[soundemittersystem.so]=''
# map[sourcevr.so]=''
# map[stdshader_dx9.so]=''
# map[studiorender.so]=''
# map[vgui2.so]=''
# map[vguimatsurface.so]=''

# map[hl2_linux]=''
# map[simdtest]=''
# map[srcds_linux]=''

# map[client.so]=''
# map[server.so]=''

echo Copying over tc2 custom files
cp -r game_clean/copy/tf/custom/tc2/ ../game/tf/custom

echo Copying over Goldberg files
cp game_clean/copy/bin/libsteam_api.so ../game/bin
cp game_clean/copy/bin/libsteamnetworkingsockets.so ../game/bin

echo Done!
