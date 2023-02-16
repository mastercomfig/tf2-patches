@echo OFF
SETLOCAL EnableDelayedExpansion
SET n=0
FOR %%l in (
            lib/public/dme_controls.lib
            lib/common/win32/2015/debug/cryptlib.lib
            lib/common/win32/2015/release/cryptlib.lib
            lib/common/x64/2015/debug/cryptlib.lib
            lib/common/x64/2015/release/cryptlib.lib
           ) do (
   SET LIBS[!n!]=%%l
   SET /A n+=1
   curl.exe https://libs.mastercomfig.com/%%l -fLo %%l --create-dirs
)