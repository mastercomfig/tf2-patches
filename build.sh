#!/usr/bin/env bash

export VALVE_NO_AUTO_P4=1
if [[ $1 == '-v' ]]; then
  make -f games.mak NO_CHROOT=1 MAKE_JOBS=1 MAKE_VERBOSE=1
else
  make -f games.mak NO_CHROOT=1
fi
