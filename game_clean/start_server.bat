@echo off
srcds.bat +sv_pure 0 +maxplayers 32 +sv_lan 1 %* < nul
