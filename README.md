# Team Comtress 2

Team Fortress 2, but with a lot of fixes, QoL improvements and performance optimizations!

## About

What is Team Comtress 2? It's a version of Team Fortress 2, based on the recent leak which aims to fix many bugs, performance issues, etc. Imagine it like mastercomfig on steroids!

Obviously, as a leaked build, it's not useful for getting better performance in Casual on its own (you can't use this build to connect to any existing servers), but it can help me a lot if you all can test it, so that I am more confident in sending many of these changes to Valve for them to include in the base game! Please let me know how it works for you!

## Install

Required depot (and manifest, optionally):

| depot | manifest |
| ----- | -------- |
| 441   | 7707612755534478338 |
| 440   | |
| 232251 | 2174530283606128348 |

1. Use [Depot Downloader](https://github.com/SteamRE/DepotDownloader) to download these depots from app `440`.
2. Combine them together into one folder.
3. [Download](https://github.com/mastercomfig/team-comtress-2/releases/latest) the latest release.
4. Extract the contents of the download to your folder, replacing existing files.
5. [Download](https://mr_goldberg.gitlab.io/goldberg_emulator/) Goldberg Emulator.
6. Extract steam_api.dll into the `bin/` folder.
7. Double click `start_tf2.bat`.
8. Enjoy!

## Build

DISCLAIMER: This is the big kids zone. If you are not a professional, building the game from source is not what you want. Use the pre-built [Releases](https://github.com/mastercomfig/team-comtress-2/releases). Also, building this on Mac/Linux, while possible, is not covered here. It might be much more complicated (or not)

### Setup
1. Get [.Net Core Runtime](https://dotnet.microsoft.com/download) (for Depot Downloader, latest version is fine)
1. Get [Visual Studio 2019 Community Edition](https://visualstudio.microsoft.com/vs/) (for building TF2)
1. Get [Depot Downloader](https://github.com/SteamRE/DepotDownloader)

### Building
1. Download this repo
1. Open `/thirdparty/protobuf-2.5.0/vsprojects/libprotobuf.vcproj`
1. When prompted to "upgrade" project, agree
1. For the Debug configuration, set `Properties->Configuration Properties->C/C++->Code Generation->Runtime Library` to `Multi-threaded Debug (/MTd)`. For Relase, set it to to `Multi-threaded (/MT)`
1. For both configurations, add `_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS` under `Properties->Configuration Properties->C/C++->Preprocessor`
1. Run both the Debug and the Release builds
1. Change file permissions to allow execution on `/thirdparty/protobuf-2.5.0/protoc.exe`
1. From the `441` depot, copy `tf/tf2_misc_dir.vpk/scripts/hudanimations_tf.txt` and `.../resource/clientscheme.res`, `.../resource/modevents.res`, and `.../resource/tf_english.txt` to the corresponding folders (`scripts` and `resource`) under `../game/tf/`. Yes, this is _outside_ the project directory
1. Run `regedit` and [fix whatever this is](https://github.com/ValveSoftware/source-sdk-2013/issues/72#issuecomment-326633328) (add a key at `HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\10.0\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}`, add a `String` property named `DefaultProjectExtension`, set the value to `vcproj`)
1. Set the [environment variable](https://superuser.com/a/985947) `VALVE_NO_AUTO_P4` to `true`
1. Change file permissions to allow execution on
  - `/devtools/bin/mc.exe`
  - `/devtools/bin/vpc.exe`
  - `/creategameprojects_dev.bat`
1. Run `/creategameprojects_dev.bat`
1. Open `/games.sln`
1. Set `Client (TF)` as the startup project in VS
1. Build the VS project
1. The executable is under `/launcher_main/Debug/default.exe` for the client and under `/dedicated_main/Debug/srcds.exe` for the server

## Legal

Valve, the Valve logo, Steam, the Steam logo, Team Fortress, the Team Fortress logo, Source, the Source logo are trademarks and/or registered trademarks of Valve Corporation in the U.S. and/or other countries.

Team Comtress 2 is not sponsored, endorsed, licensed by, or affiliated with Valve Corporation.

See license for details.
