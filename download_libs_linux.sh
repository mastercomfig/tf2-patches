#!/bin/bash
declare -a libs=("lib/common/linux64/libcryptopp.a" "tools/runtime/steamrt_scout_amd64.tar.xz" "tools/runtime/steamrt_scout_i386.tar.xz")
server="https://libs.mastercomfig.com/"

for lib in "${libs[@]}"
do
  curl "${server}${lib}" -Lo "${lib}" --create-dirs
done
