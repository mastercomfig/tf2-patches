#!/usr/bin/env bash
GAME_DIR="./game"

# NOTE: Do not set -e, some cp commands fail because they don't overwrite existing files, which we want
cd "$(dirname "$0")" || exit 1

if [[ -n "$1" ]]; then
	ORIGINAL_INSTALL_DIR="$1"
else
	ORIGINAL_INSTALL_DIR="${HOME}/.steam/steam/steamapps/common/Team Fortress 2"
fi

if [[ ! -d "$ORIGINAL_INSTALL_DIR" ]]; then
    echo Failed to find TF2 at \'"${ORIGINAL_INSTALL_DIR}"\'
    echo Did you move your steam library?
    echo If so, pass the path to your TF2 installation after \'link.sh\'
    echo If you already did so, check your supplied path
    exit 1
fi

link_dir() {
	ln -sn "${ORIGINAL_INSTALL_DIR}/$1" "$GAME_DIR/$1"
}

link_glob() {
	ln -sn "${ORIGINAL_INSTALL_DIR}/$1/"*"$2" "$GAME_DIR/$1/"
}

copy () {
	cp -rfT --remove-destination "${ORIGINAL_INSTALL_DIR}/$1" "$GAME_DIR/$1"
}

git submodule update --init

cp -rfT game_clean/copy/ $GAME_DIR/

link_dir hl2
link_dir platform
link_dir tf/maps
mkdir $GAME_DIR/tf/materials
link_dir tf/materials/models
link_dir tf/materials/vgui
link_dir tf/media
link_dir tf/resource
link_dir tf/scripts

link_glob bin '' 2> >(grep -v "File exists")
link_glob tf '.vpk'
link_glob tf '.cache'

copy tf/cfg
copy tf/gamestate.txt
copy tf/glshaders.cfg
copy tf/videoconfig_linux.cfg