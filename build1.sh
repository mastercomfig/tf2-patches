#!/usr/bin/env bash

if [[ -z $1 ]]; then
  echo 'Usage: build1.sh <project> [extra make args...]'
fi

export VALVE_NO_AUTO_P4=1
if [[ $1 =~ ^client|server$ ]]; then
  cd game/"$1" && make -f "$1_linux32_tf.mak" NO_CHROOT=1 "${@:2}"
else
  cd "$1" && make -f "$1_linux32.mak" NO_CHROOT=1 "${@:2}"
fi
