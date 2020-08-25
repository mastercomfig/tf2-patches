[![ko-fi](https://img.shields.io/badge/Support%20me%20on-Ko--fi-FF5E5B.svg?logo=ko-fi&style=flat-square)](https://ko-fi.com/W7W510R1Q)
[![Liberapay](https://img.shields.io/liberapay/receives/mastercoms.svg?logo=liberapay&style=flat-square)](https://liberapay.com/mastercoms/)
[![Steam donate](https://img.shields.io/badge/Donate%20via-Steam-00adee.svg?style=flat-square&logo=steam)](https://steamcommunity.com/tradeoffer/new/?partner=85845165&token=M9cQHh8N)
[![Join the Discord chat](https://img.shields.io/badge/Discord-%23comtress--client-7289da.svg?style=flat-square&logo=discord)](https://discord.gg/CuPb2zV)


# Team Comtress 2

Team Fortress 2, but with a lot of fixes, quality of life improvements and performance optimizations!

## About

What is Team Comtress 2? It's a version of Team Fortress 2, based on the recent leak which aims to fix many bugs, performance issues, etc. Imagine it like mastercomfig on steroids!

Obviously, as a leaked build, it's not useful for getting better performance in Casual on its own (you can't use this build to connect to any existing servers), but it can help me a lot if you all can test it, so that I am more confident in sending many of these changes to Valve for them to include in the base game! Please let me know how it works for you!

## Install

1. [Download](https://github.com/mastercomfig/team-comtress-2/releases/latest) the latest release.
2. Copy your current `Team Fortress 2` installation to a new folder.
3. Extract the ZIP download to this new folder.
4. Double click `start_tf2.bat`. Note that you must have Steam running.
5. Enjoy!

## Build

**DISCLAIMER:** If you are not a developer, building the game from source is not what you want. Use the pre-built [Releases](https://github.com/mastercomfig/team-comtress-2/releases). Also, building this on Mac/Linux, while possible, is not covered here. Please let us know if you get it to work!

### Building
1. Clone this repo
2. Open `/thirdparty/protobuf-2.5.0/vsprojects/libprotobuf.vcproj`
3. Run both the Debug and the Release builds
4. Run `regedit` and [fix whatever this is](https://github.com/ValveSoftware/source-sdk-2013/issues/72#issuecomment-326633328) (add a key at `HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\10.0\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}`, add a `String` property named `DefaultProjectExtension`, set the value to `vcproj`)
5. Set the [environment variable](https://superuser.com/a/985947) `VALVE_NO_AUTO_P4` to `true` and `PreferredToolArchitecture` to `x64`.
6 Run `/creategameprojects_dev.bat`
7. Open `/games.sln`
8. Build the VS project
9. The executables are placed at `../game/hl2.exe` for the client and at `../game/srcds.exe` for the server. Note: this path is outside the repository.

### Running and Debugging
1. For the compiled binaries to run, you will need to copy your current TF2 installation to `../game` (relative to your repostiory, outside of it).
2. To setup debugging, in Visual Studio, select `Client (TF)` as the startup project, then go to its `Properties->Configuration Properties->Debugging`. Set `Command` to your `../game/hl2.exe` binary, the `Command Arguments` to `-steam -game tf -insecure -novid -nojoy -nosteamcontroller -nohltv -particles 1 -noborder -particle_fallback 2 -dev -allowdebug` and `Working Directory` to your game installation folder i.e. `../game/bin`. Note: all the paths here are relative to your copy of the repository (same place where `games.sln` is located), do **not** set these values verbatim.
3. For server, follow the same procedures but choose the `Server (TF)` project and set the `Command` to `../game/srcds.exe`. The suggested server launch options are `-game tf -console -nomaster -insecure +sv_pure 0 +maxplayers 32 +sv_lan 1 -dev -allowdebug`.

NOTE: Team Comtress 2 is no longer compatible with mastercomfig. Please do not use mastercomfig or any other TF2 config.

See [the Valve dev wiki page](https://developer.valvesoftware.com/wiki/Installing_and_Debugging_the_Source_Code) for another explanation of the last two steps.

Other launch options to consider:
- `sw` to force windowed mode
- `-w WIDTH -h HEIGHT` to set the resolution
- `+map MAPNAME` to automatically launch a map on startup

## Legal

Valve, the Valve logo, Steam, the Steam logo, Team Fortress, the Team Fortress logo, Source, the Source logo are trademarks and/or registered trademarks of Valve Corporation in the U.S. and/or other countries.

Team Comtress 2 is not sponsored, endorsed, licensed by, or affiliated with Valve Corporation.

See license for details.
