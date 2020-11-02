${ErrorActionPreference}  = "Stop"

${server}                 = "https://libs.mastercomfig.com/"
# $IsWindows was added in PowerShell 6.0, so allow it to be null.
If (( ${IsWindows} ) -or
    ( ${IsWindows} -eq $null )) {
        ${file_extension} = ".exe"
        ${libraries}      = @(
            "lib/public/dme_controls.lib"               ;
            "lib/common/win32/2015/debug/cryptlib.lib"  ;
            "lib/common/win32/2015/release/cryptlib.lib";
            "lib/common/x64/2015/debug/cryptlib.lib"    ;
            "lib/common/x64/2015/release/cryptlib.lib"  ;
            "lib/common/x64/2015/debug/cryptlib.lib"    )
}ElseIf ( ${IsLinux} ) {
    ${libraries}          = @(
        "lib/common/linux64/libcryptopp.a"              ;
        "tools/runtime/linux/steamrt_scout_amd64.tar.xz";
        "tools/runtime/linux/steamrt_scout_i386.tar.xz" )
}ElseIf ( ${IsmacOS} ) {
    ${libraries}          = @(
        "lib/common/osx32/libcryptopp.a"                )
}
# curl used to be aliased to Invoke-WebRequest, so add the file extension to avoid conflicts.
If ( Get-Command -ErrorAction SilentlyContinue curl${file_extension} ) {
    ForEach ( ${library} in ${libraries} ) {
        curl${file_extension} ${server}${library} -fLo ${library} --create-dirs
    }
}ElseIf ( Get-Command -ErrorAction Continue Invoke-WebRequest ) {
    ForEach ( ${library} in ${libraries} ) {
        Invoke-WebRequest ${server}${library} -OutFile ${library}
    }
}Else {
    Throw "Invoke-WebRequest not found. This is likely due to using an outdated version of PowerShell."
}
