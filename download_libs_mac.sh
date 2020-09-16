#!/bin/bash
declare -a libs=("lib/common/osx32/libcryptopp.a")
server="https://libs.mastercomfig.com/"

for lib in "${libs[@]}"
do
  curl "${server}${lib}" -o "${lib}"
done
