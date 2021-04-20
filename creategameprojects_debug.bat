@echo OFF
msbuild /m /p:Platform="x86";Configuration=Release external/vpc/vpc.sln
devtools\bin\vpc.exe /2015 /define:WORKSHOP_IMPORT_DISABLE /define:SIXENSE_DISABLE /define:NO_X360_XDK /define:RAD_TELEMETRY_DISABLED /define:DISABLE_ETW /no_ceg /nofpo /tf +game /mksln games.sln
