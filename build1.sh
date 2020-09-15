#!/usr/bin/env bash

export VALVE_NO_AUTO_P4=1
cd "$1" && make -f "$1_linux32.mak" NO_CHROOT=1 "${@:2}"
