#!/usr/bin/env bash

./creategameprojects_dev.sh
sed -i 's/\/bin\/bash/bash/g' games.mak
