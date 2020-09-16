@echo off
srcds.bat +sv_pure 1 +maxplayers 32 +sv_lan 1 %* < nul
