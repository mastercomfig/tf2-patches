@echo off
setlocal EnableDelayedExpansion

set df=DepotDownloader\DepotDownloader.dll
set dd="dotnet %df%"
set dp=depots

echo.
echo This will download compatible files from an old version of Team Fortress 2 from Steam using your account.
echo.

WHERE dotnet >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
	echo DepotDownloader requires .NET Core Runtime, which you can download here: https://dotnet.microsoft.com/download
	echo.
	PAUSE
	exit 1
)

if exist %df% (
	rem exists
) else (
	echo Please download DepotDownloader from https://github.com/SteamRE/DepotDownloader/releases/latest and place it in a folder called DepotDownloader.
	echo.
	PAUSE
	exit 2
)

set /p steam_username="Steam Username: "
set /p steam_password="Steam Password: "

set passes=true
if %passes% equ true (
	"%dd%" -app 440 -depot 440
	"%dd%" -app 440 -depot 232251 -manifest 2174530283606128348 -username %steam_username% -password %steam_password%
	"%dd%" -app 440 -depot 441 -manifest 7707612755534478338 -username %steam_username% -password %steam_password%
	robocopy "%dp%\440\5249752" . /E /XC /XN /XO /XD .DepotDownloader /MOV
	robocopy "%dp%\232251\5249752" . /E /XC /XN /XO /XD .DepotDownloader /MOV
	robocopy "%dp%\441\5249752" . /E /XC /XN /XO /XD .DepotDownloader /MOV
)

PAUSE
