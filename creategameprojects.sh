#!/usr/bin/env bash

chmod u+x ./devtools/bin/vpc
devtools/bin/vpc /define:WORKSHOP_IMPORT_DISABLE /define:SIXENSE_DISABLE /define:NO_X360_XDK /define:RAD_TELEMETRY_DISABLED /define:DISABLE_ETW /define:LTCG /retail /define:CERT /tf +game /mksln games
