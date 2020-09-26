#!/usr/bin/env sh

export LD_LIBRARY_PATH="/usr/lib32:./bin:./tf/bin:${LD_LIBRARY_PATH}"
echo "LD_LIBRARY_PATH = ${LD_LIBRARY_PATH}"

mango="$(command -v mangohud)"

if [ "$1" = '-d' ]; then
  cd ../game && lldb ./hl2_linux -- -steam -game tf -insecure -novid -nojoy -nosteamcontroller -nohltv -particles 1 -noborder -particle_fallback 2 -dev -allowdebug
else
  cd ../game && MANGOHUD_DLSYM=1 $mango ./hl2_linux -steam -game tf -insecure -novid -nojoy -nosteamcontroller -nohltv -particles 1 -noborder -particle_fallback 2 -dev -allowdebug
fi
