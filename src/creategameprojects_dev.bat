@echo OFF
MSBuild.exe /?
if ERRORLEVEL 1 (
  echo MsBuild not found in PATH. Please, start from Developer Command Prompt or add MSVC MsBuild directory to the PATH.
  exit /B 1
)

MSBuild.exe /m /p:Platform="x86" /p:Configuration=Release external/vpc/vpc.sln
devtools\bin\vpc.exe /2015 /define:WORKSHOP_IMPORT_DISABLE /define:SIXENSE_DISABLE /define:NO_X360_XDK /define:RAD_TELEMETRY_DISABLED /define:DISABLE_ETW /define:LTCG /no_ceg /retail /define:CERT /nofpo /hl2 +game /mksln games.sln
