@echo OFF

where msbuild.exe > nul 2>&1 || echo msbuild.exe not found. Read the Windows build instructions!
where devenv > nul 2>&1 || echo devenv not found. Read the Windows build instructions!

set PROTOBUF_PATH=thirdparty\protobuf-2.5.0\vsprojects
IF NOT EXIST %PROTOBUF_PATH%\Release\libprotobuf.lib (
     IF NOT EXIST %PROTOBUF_PATH%\Debug\libprotobuf.lib (
        msbuild /m /p:Configuration=Debug   %PROTOBUF_PATH%\libprotobuf.vcxproj
        msbuild /m /p:Configuration=Release %PROTOBUF_PATH%\libprotobuf.vcxproj
    )
)

set CRYPTOLIB_PATH=external\crypto++-5.6.3
IF NOT EXIST %CRYPTOLIB_PATH%\cryptest.sln (
    powershell -Command Expand-Archive -Force %CRYPTOLIB_PATH%\vs2010.zip %CRYPTOLIB_PATH%
    devenv /Upgrade %CRYPTOLIB_PATH%\cryptest.sln
)

IF NOT EXIST %CRYPTOLIB_PATH%\Win32\Output\Release\cryptlib.lib (
    IF NOT EXIST %CRYPTOLIB_PATH%\Win32\Output\Debug\cryptlib.lib (
        msbuild /m /t:cryptlib /p:Configuration=Debug   %CRYPTOLIB_PATH%\cryptest.sln
        msbuild /m /t:cryptlib /p:Configuration=Release %CRYPTOLIB_PATH%\cryptest.sln
    )
)

IF NOT EXIST lib\common\win32\2015\release\cryptlib.lib (
    IF NOT EXIST lib\common\win32\2015\debug\cryptlib.lib (
        rem msbuild /m /t:cryptlib /p:"Configuration=Debug;Platform=x64"   %CRYPTOLIB_PATH%\cryptest.sln
        rem msbuild /m /t:cryptlib /p:"Configuration=Debug;Platform=x64"   %CRYPTOLIB_PATH%\cryptest.sln
        robocopy %CRYPTOLIB_PATH%\Win32\Output\Release lib\common\win32\2015\release cryptlib.lib /r:0 /w:0
        robocopy %CRYPTOLIB_PATH%\Win32\Output\Debug   lib\common\win32\2015\debug   cryptlib.lib /r:0 /w:0
        rem robocopy %CRYPTOLIB_PATH%\x64\Output\Release lib\common\x64\2015\release cryptlib.lib /r:0 /w:0
        rem robocopy %CRYPTOLIB_PATH%\x64\Output\Debug   lib\common\x64\2015\debug   cryptlib.lib /r:0 /w:0
        IF %ERRORLEVEL% LEQ 8 EXIT /B 0
    )
)
