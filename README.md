# Team Comtress 2

Team Fortress 2, but with a lot of fixes, QoL improvements and performance optimizations!

## About

What is Team Comtress 2? It's a version of Team Fortress 2, based on the recent leak which aims to fix many bugs, performance issues, etc. Imagine it like mastercomfig on steroids!

Obviously, as a leaked build, it's not useful for getting better performance in Casual on its own (you can't use this build to connect to any existing servers), but it can help me a lot if you all can test it, so that I am more confident in sending many of these changes to Valve for them to include in the base game! Please let me know how it works for you!

## Install

1. [Download](https://github.com/mastercomfig/team-comtress-2/releases/latest) the latest release.
2. Extract to a folder of your choosing.
3. Run `download_depots.bat`.
5. Double click `start_tf2.bat`. Note that you must have Steam running.
6. Enjoy!

## Build

DISCLAIMER: This is the big kids zone. If you are not a professional, building the game from source is not what you want. Use the pre-built [Releases](https://github.com/mastercomfig/team-comtress-2/releases). Also, building this on Mac/Linux, while possible, is not covered here. It might be much more complicated (or not)

### Setup
1. Get [.NET Core Runtime](https://dotnet.microsoft.com/download) for Depot Downloader, latest version is fine.
1. Get [Visual Studio 2019 Community Edition](https://visualstudio.microsoft.com/vs/) for building TF2. The required installation components are: "Desktop development with C++" and the "C++ MFC for latest v142 build tools (x86 & x64)".
1. Get [Depot Downloader](https://github.com/SteamRE/DepotDownloader).

### Depot downloader
The preferred way for downloading game depots is using `/game_clean/download_depots.bat`, it will guide you through the process.

Alternatively, see manual download instructions below.

### Building
1. Download this repo
1. Open `/thirdparty/protobuf-2.5.0/vsprojects/libprotobuf.vcproj`
1. Run both the Debug and the Release builds
1. Run `regedit` and [fix whatever this is](https://github.com/ValveSoftware/source-sdk-2013/issues/72#issuecomment-326633328) (add a key at `HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\10.0\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}`, add a `String` property named `DefaultProjectExtension`, set the value to `vcproj`)
1. Set the [environment variable](https://superuser.com/a/985947) `VALVE_NO_AUTO_P4` to `true`
1. Run `/creategameprojects_dev.bat`
1. Open `/games.sln`
1. Build the VS project
1. The executables are placed at `../game/hl2.exe` for the client and at `../game/srcds.exe` for the server. Note: this path is outside the repository.

### Running and Debugging
1. For the compiled binaries to run, you will need all (some, but we are not sure which exactly) of the `.dll` and `.asi` files under `bin`. Copy them from the corresponding folder of depot `232251` to your game installation i.e. `../game/bin` (yes, this is _outside_ the project directory). Make sure you do not override any of your locally built DLLs
1. You will also need all the usual game resources (same as when installing a pre-built release). Feel free to skip `.sound.cache` files, but otherwise just merge all the depots into `../game`. Once again, do not override any files that VS put in `../game/bin`
1. To setup debugging, in Visual Studio, select `Client (TF)` as the startup project, then go to its `Properties->Configuration Properties->Debugging`. Set `Command` to your `../game/hl2.exe` binary, the `Command Arguments` to `-steam -game tf -insecure -novid -nojoy -nosteamcontroller -nohltv -particles 1 -noborder -particle_fallback 2 -dev -allowdebug` and `Working Directory` to your game installation folder i.e. `../game/bin`. Note: all the paths here are relative to your copy of the repository (same place where `games.sln` is located), do **not** set these values verbatim.
1. For server, follow the same procedures but set the `Command` to `../game/srcds.exe`. Suggest server launch options are `-game tf -console -nomaster -insecure +sv_pure 0 +maxplayers 32 +sv_lan 1 -dev -allowdebug`.

See [the Valve dev wiki page](https://developer.valvesoftware.com/wiki/Installing_and_Debugging_the_Source_Code) for another explanation of the last two steps.

Other launch options to consider:
- `sw` to force windowed mode
- `-w WIDTH -h HEIGHT` to set the resolution
- `+map MAPNAME` to automatically launch a map on startup

### Manual Depot Download
1. Open `cmd.exe` (Windows-R + type cmd.exe + enter) or any other shell
1. Type `cd ` and drag the Depot Downloader folder (unzipped) to the window
1. Hit enter
1. Run the following, where `USERNAME` is your Steam username and `PASSWORD` is your Steam password:
    - `dotnet DepotDownloader.dll -app 440 -depot 441 -manifest 7707612755534478338 -username USERNAME -password PASSWORD`
    - `dotnet DepotDownloader.dll -app 440 -depot 440 -username USERNAME -password PASSWORD`
    - `dotnet DepotDownloader.dll -app 440 -depot 232251 -manifest 2174530283606128348 -username USERNAME -password PASSWORD`

If prompted for Steam Guard code, enter it.

## Build System Info

### Configuration Separation
To ensure that Release and Debug builds do not conflict, we replaced the default `$LIBPUBLIC` macro with two: `$LIBPUBLICDEBUG` and `$LIBPUBLICRELEASE`. Same for libraries that get generated into `$LIBCOMMON`. This means that most builds will not work by default since they need `$LIBPUBLIC` to be defined. This includes the `$Lib` and `$ImpLib` VPC commands. Fixing this involves going through all the occurances of `$LIBPUBLIC` and `$LIBCOMMON` (if it is a generated library and not imported one) and replacing them with two `$File` commands (one for debug and one for release) with `$ExcludedFromBuild "Yes"` in the configuration for the opposite configuration (i.e. for `$LIBPUBLICDEBUG` set `$ExcludedFromBuild` in the `"Release"` configuration).

Since this is extremely non-trivial,
```
$Macro ARGLIBNAME library_name
$Include "$SRCDIR\vpc_scripts\lib_include.vpc"
```
will do it for you for the `library_name` library.

A similar script for `$LIBCOMMON` libraries is `$SRCDIR\vpc_scripts\lib_common_include.vpc`

### Published DLL Tracking
In Valve's build scripts, the runtime DLL dependencies are implicit, so if a required DLL is missing or needs a rebuild, it will not be picked up by the build system. Fixing this invovles adding project dependencies with
```
$Macro ARGPROJNAME proj_name
$Include "$SRCDIR\vpc_scripts\proj_include.vpc"
```

To find which projects you need, run the `/clean_bins.sh` script (MinGW or Cygwin required) to clean out the published binaries and cause the game to print errors whenever a dependency is missing. Add each of these dependencies and the builds should work.

To avoid going through a VPC re-run and a VS rebuild that follows for the core project, build the dependencies manually first and check if any others are needed before adding them to the VPC. This saves a lot of time.

### PyVPC
A WIP Python port of the Valve VPC tool is included under `/pyvpc/`. I have only tested it from under Cygwin, under which Python gets very confused regarding which platform it is running on, so some hacks were necessary. It should probably still work under native Windows. It currently has a hard dependcy on the [`colorful`](https://github.com/timofurrer/colorful) module.

The plan for this tool is to have a way of parsing the original Valve VPC files, then applying patches to the generated configurations from a Python script, then using that representation to generate `.vcproj` files. Or we might juse use the Valve configurations as a base to manually write Python build definitions.

In any case, the processing of the VPC files is there for greater compatibility with potential future leaks/working with other code based on VPC.

We do not want to use VPC ultimately, since it depends on a large amount of Valve code (its own version of tier0, vstdlib, and more) and because patching C tools is a pain. And we do want to patch it because of the various bugs and inflexibilities in VPC which make some things outright impossible/not work, and others extremely complicated (see the two sections above). The patch for clean build configuration separation alone touched 71 files and required more than 1000 changes.

## Legal

Valve, the Valve logo, Steam, the Steam logo, Team Fortress, the Team Fortress logo, Source, the Source logo are trademarks and/or registered trademarks of Valve Corporation in the U.S. and/or other countries.

Team Comtress 2 is not sponsored, endorsed, licensed by, or affiliated with Valve Corporation.

See license for details.
