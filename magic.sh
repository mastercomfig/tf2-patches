#!/usr/bin/env bash

set -e

echo This will magically build the project in the future.
echo Right now it is woefully incomplete. No support will be provided.
echo
echo Use -f to override
force=0
verbose=0
for arg in "$@"; do
  if [[ ${arg} == -vf || ${arg} == -fv ]]; then
    force=1
    verbose=1
  fi
  if [[ ${arg} == -v ]]; then
    verbose=1
  fi
  if [[ ${arg} == -f ]]; then
    force=1
  fi
done
if [[ ${force} != 1 ]]; then exit 1; fi
echo
echo Proceeding anyway! Hope you know what you are doing!

if [[ ! -d ../game ]]; then
  echo Error: no installation folder found.
  echo Install TF2 resources to ../game before proceding!
  echo 'Otherwise build products will get overwritten when you do this later.'
  exit 1
fi

# Really wish git did not ruin our permissions
echo Making shell scripts executable
chmod u+x vpc.sh
chmod u+x build.sh
chmod u+x build1.sh
chmod u+x run.sh

if [[ ${verbose} == 1 ]]; then
  exec 3>&1 4>&2
else
  exec 3>&1 4>&2 &>/dev/null
fi

cflags='-D_GLIBCXX_USE_CXX11_ABI=0'
configure_args=('--host=i686-unknown-linux-gnu' "CFLAGS=\"${cflags}\"" "CXXFLAGS=\"${cflags}\"" '--enable-shared=no')

function makethirdparty() {
  chmod u+x ./configure && ./configure "${configure_args[@]}" && make
}

echo Building Gperftools 1>&3 2>&4
(cd thirdparty/gperftools-2.0/ && makethirdparty)

echo Building protobuf 1>&3 2>&4
(cd thirdparty/protobuf-2.5.0/ && makethirdparty)

echo Building libedit 1>&3 2>&4
(cd thirdparty/libedit-3.1/ && makethirdparty)

echo Running VPC 1>&3 2>&4
./vpc.sh

echo Building games.mak 1>&3 2>&4
chmod u+x devtools/gendbg.sh 1>&3 2>&4
./build.sh

echo Done!
