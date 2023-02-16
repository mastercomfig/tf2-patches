#!/bin/bash

set -e

if [[ "$OSTYPE" != "darwin"* ]]; then    # Linux
  declare -a LIBS=(
                    lib/common/linux64/libcryptopp.a
                    tools/runtime/linux/steamrt_scout_{amd64,i386}.tar.xz
                  )
elif [[ "$OSTYPE" == "darwin"* ]]; then  # Mac
  declare -a LIBS=(
                    lib/common/osx32/libcryptopp.a
                  )
else; then  # Windows
  declare -a LIBS=(
                    lib/public/dme_controls.lib
                    lib/common/win32/2015/debug/cryptlib.lib
                    lib/common/win32/2015/release/cryptlib.lib
                    lib/common/x64/2015/debug/cryptlib.lib
                    lib/common/x64/2015/release/cryptlib.lib
                  )
fi

for LIB in "${LIBS[@]}"
do
  curl "https://libs.mastercomfig.com/${LIB}" -fLo "${LIB}" --create-dirs
done