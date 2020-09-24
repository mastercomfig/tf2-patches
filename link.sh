#!/usr/bin/env sh
cd "$(dirname "$0")" || exit

if [ -z "$1" ]; then
    live_install="$HOME/.steam/steam/steamapps/common/Team Fortress 2/"
else
	live_install="$1"
fi

if [ ! -d "$live_install" ]; then
    echo Failed to find TF2 at \'"$live_install"\'
    echo Did you move your steam library?
    echo If so, put the path to your TF2 folder after \'link.sh\'
    exit 1
fi

link_dir() {
	ln -sn "$live_install/$1" "../game/$1"
}

link_glob() {
	ln -sn "$live_install/$1/"*"$2" "../game/$1/"
}

copy () {
	cp -rfT --remove-destination "$live_install/$1" "../game/$1"
}

link_dir hl2
link_dir platform
link_dir tf/maps
mkdir ../game/tf/materials
link_dir tf/materials/models
link_dir tf/materials/vgui
link_dir tf/media
link_dir tf/resource
link_dir tf/scripts

link_glob bin ''
link_glob tf '.vpk'
link_glob tf '.cache'   # IDK why we need this, but we crash on CAudioSourceMemWave (...): GetDataPointer() without it

copy tf/cfg
copy tf/gameinfo.txt
copy tf/gamestate.txt
copy tf/steam.inf
copy tf/glshaders.cfg
copy tf/videoconfig_linux.cfg

cp -f --remove-destination game_clean/copy/bin/*.so ../game/bin/
cp -r game_clean/copy/tf/custom/ ../game/tf/custom
