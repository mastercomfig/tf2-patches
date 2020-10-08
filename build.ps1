$ErrorActionPreference = "Stop"

#TODO: Deduplicate this with ForEach or something
$error_msg = "is not defined. See https://github.com/mastercomfig/team-comtress-2/wiki/Windows-Build-Instructions#building"
If (!$env:VALVE_NO_AUTO_P4) {
    Throw "VALVE_NO_AUTO_P4 ${error_msg}"
}
If ($env:PreferredToolArchitecture -ne "x64") {
    Throw "PreferredToolArchitecture ${error_msg}"
}
If ((gp "HKLM:SOFTWARE\WOW6432Node\Microsoft\VisualStudio\10.0\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}").DefaultProjectExtension -ne "vcproj") {
    Throw "vcproj regkey ${error_msg}"
}
#TODO: Check for VS dependencies

# Check if libs have been downloaded
If ((!(Test-Path lib\public\dme_controls.lib))                 -or
    (!(Test-Path lib\common\win32\2015\debug\cryptlib.lib)))   -or
    (!(Test-Path lib\common\win32\2015\release\cryptlib.lib))) -or
    (!(Test-Path lib\common\x64\2015\debug\cryptlib.lib)))     -or
    (!(Test-Path lib\common\x64\2015\release\cryptlib.lib)))     {
        download_libs.bat
}

If (!(Test-Path vswhere.exe)) { #Maybe use Invoke-WebRequest?
    curl -fLO https://github.com/microsoft/vswhere/releases/latest/download/vswhere.exe
}
# https://github.com/microsoft/vswhere/wiki/Start-Developer-Command-Prompt#using-powershell
$installationPath = (.\vswhere.exe -prerelease -latest -property installationPath)
If ($installationPath -and (Test-Path "$installationPath\Common7\Tools\vsdevcmd.bat")) {
  & "${env:COMSPEC}" /s /c "`"$installationPath\Common7\Tools\vsdevcmd.bat`" -no_logo && set" | ForEach-Object {
    $name, $value = $_ -split '=', 2
    Set-Content env:\"$name" $value
  }
}

If ((!(Test-Path thirdparty\protobuf-2.5.0\vsprojects\Debug\*.lib)) -or
    (!(Test-Path thirdparty\protobuf-2.5.0\vsprojects\Release\*.lib))) {
        $libprotobuf_path = thirdparty\protobuf-2.5.0\vsprojects
        msbuild ${libprotobuf_path}\libprotobuf.vcxproj /p:Configuration=Release
        msbuild ${libprotobuf_path}\libprotobuf.vcxproj /p:Configuration=Debug
}

.\creategameprojects_dev.bat
#TODO: Check if property is already present
# Set C(++) standard to 17 in a hacky way
Get-ChildItem -r -include "*.vc*proj" |
ForEach { $a = $_.fullname; ( Get-Content $a ) |
ForEach { $_ -replace "</ClCompile>","`n      <LanguageStandard>stdcpp17</LanguageStandard>`n      <LanguageStandard_C>stdc17</LanguageStandard_C>`n    </ClCompile>" }  | 
Set-Content $a }
# sln needs to be upgraded to work with msbuild
#TODO: Do same thing as Momentum mod since it's faster
Start-Process "devenv.com" "games.sln /upgrade" -NoNewWindow -Wait

msbuild games.sln /m /p:configuration=Release
