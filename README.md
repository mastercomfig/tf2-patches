
[![ko-fi](https://img.shields.io/badge/Support%20me%20on-Ko--fi-FF5E5B.svg?logo=ko-fi&style=flat-square)](https://ko-fi.com/mastercoms)
[![Liberapay](https://img.shields.io/liberapay/receives/mastercoms.svg?logo=liberapay&style=flat-square)](https://liberapay.com/mastercoms/)
[![Steam donate](https://img.shields.io/badge/Donate%20via-Steam-00adee.svg?style=flat-square&logo=steam)](https://steamcommunity.com/tradeoffer/new/?partner=85845165&token=M9cQHh8N)
[![Join the Discord chat](https://img.shields.io/badge/Discord-%23comtress--client-7289da.svg?style=flat-square&logo=discord)](https://discord.gg/CuPb2zV)


# Welcome to Team Comtress 2!
Before sharing your ideas and feedback, it's important to have a good understanding of what this project is.
### What *is* TC2?
Team Comtress 2 is a fork of an older version of Team Fortress 2, with a community developer team fixing bugs, improving performance, and adding quality of life features, with the goal of having those changes pulled upstream by Valve to the modern game.

### What is TC2 *not*?
* **TC2 is *not* a "pro mod"**

While much of the work in TC2 will improve the competitive experience, TC2 does not touch gameplay, balance, or competitive rules.

* **TC2 is *not* a vision for the "good old days" of TF2**

TC2 will not feature game design changes, gameplay additions, or almost any form of new content. If you're looking for this kind of content, [Team Fortress 2 Classic](https://tf2classic.com/) is more up your alley.

* **TC2 is *not* the modern game, with new content/updates**

You're looking for [Creators.TF](https://creators.tf/).

* **TC2 is *not* an alternative, more stable game experience**

TC2 is based on the 2017 TF2 source code leak, which contained an in-development version of Jungle Inferno. This build is rife with issues and incomplete features, and *is not compatible with* the modern game client. It isn't even fully playable on its own.

## Installation

**Windows-only for now** (see [#2](https://github.com/mastercomfig/team-comtress-2/issues/2))

1. Copy your current `Team Fortress 2` installation to a new folder.
2. In your copied TF2 folder, delete the `tf/custom` folder, as well as `tf/cfg/autoexec.cfg`, `tf/cfg/config.cfg`, and any class configs (such as `heavyweapons.cfg` or `spy.cfg`).
3. [Download](https://github.com/mastercomfig/team-comtress-2/releases/latest) `game_clean.zip` from the latest release.
4. Extract the downloaded ZIP to your copied TF2 folder, overwriting existing files.
5. Double click `start_tf2.bat`. Note that you must have Steam running.
6. Enjoy!

Note: TC2 bundles the stock HUD with [CriticalFlaw's HUD fix](https://github.com/CriticalFlaw/TF2-HUD-Fix) in the custom folder

## New console commands and launch options

Although configs are *not* recommended (use video options to customize), there *are* some new variables you can try out that haven't been added to the in-game options yet!

**Console commands/variables:**

* `tf_taunt_first_person_enable`: Forces first person taunts
* `tf_viewmodels_offset_override`: Unlocked from base TF2, format is x y z
* `tf_disable_weapon_skins`: Disables skins
* `tf_skip_halloween_bomb_hat_translucency`: Halloween bomb hat will disappear if spy cloaks, instead of turning translucent along with cloak
* `r_skybox_lowend`: Use low quality skybox textures only meant for DX8
* `tf_hud_target_id_disable`: Disable searching for a player to show the target ID for
* `tf_viewmodel_alpha`: Controls how translucent viewmodels are (1-255)
* `dsp_off`: Unlocked from base TF2, disables sound positional effects
* `cl_ragdoll_disable`: Disables all corpse effects (gibs, disintegration, ragdolls)
* `tf_fx_blood`: Controls blood splatter effects
* `fx_drawimpactdebris`, `fx_drawimpactdust`, `fx_drawmetalspark`: Unlocked from base TF2, controls bullet impact dust
* `cl_hud_playerclass_playermodel_lod`: Controls LOD for the player model preview in the HUD
* `g_ragdoll_fadespeed`, `g_ragdoll_lvfadespeed`: Controls how fast ragdolls fade (lv is for low violence mode)
* `cl_particle_retire_cost`: Unlocked from base TF2, set to `0.0001` to force lower quality particles
* `r_force_fastpath 1`: Forces shader fast paths for higher GPU performance.
* `tf_weaponspread_continuous_seed`: If set to >-1, the base seed for fixed recoil spread for continuo
us single bullet fire weapons.
* `tf_weaponspread_continuous_seed_multishot`: If set to >-1, the base seed for fixed recoil spread for continuous multi-bullet fire weapons like the Minigun.

**Launch options:**

* `-particle_fallback`: 2 uses DX8 particles, 1 uses lowend DX9 particles, 0 uses default. `start_tf2.bat` launches with `-particle_fallback 2` by default.

## Build

**DISCLAIMER:** If you are not a developer, building the game from source is not what you want. Use the pre-built [Releases](https://github.com/mastercomfig/team-comtress-2/releases). Also, building this on Mac/Linux, while possible, is not covered here. Please let us know if you get it to work!

### Building
1. Get [Visual Studio 2019 Community Edition](https://visualstudio.microsoft.com/vs/) for building TF2. The required installation components are: "Desktop development with C++" and the "C++ MFC for latest v142 build tools (x86 & x64)".
2. Clone this repo
3. Open `/thirdparty/protobuf-2.5.0/vsprojects/libprotobuf.vcxproj`
4. Run both the Debug and the Release builds
5. Run `regedit` and [add an association for the latest VS](https://github.com/ValveSoftware/source-sdk-2013/issues/72#issuecomment-326633328) (add a key at `HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\10.0\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}`, add a `String` property named `DefaultProjectExtension`, set the value to `vcproj`)
6. Set the [environment variable](https://superuser.com/a/985947) `VALVE_NO_AUTO_P4` to `true` and `PreferredToolArchitecture` to `x64`.
7. Run `/creategameprojects_dev.bat`
8. Open `/games.sln`
9. Build the VS project
10. The executables are placed at `../game/hl2.exe` for the client and at `../game/srcds.exe` for the server. Note: this path is outside the repository.

### Running and Debugging
1. For the compiled binaries to run, you will need to copy your current TF2 installation to `../game` (relative to your repostiory, outside of it).
2. To setup debugging, in Visual Studio, select `Client (TF)` as the startup project, then go to its `Properties->Configuration Properties->Debugging`. Set `Command` to your `../game/hl2.exe` binary, the `Command Arguments` to `-steam -game tf -insecure -novid -nojoy -nosteamcontroller -nohltv -particles 1 -noborder -particle_fallback 2 -dev -allowdebug` and `Working Directory` to your game installation folder i.e. `../game/bin`. Note: all the paths here are relative to your copy of the repository (same place where `games.sln` is located), do **not** set these values verbatim.
3. For server, follow the same procedures but choose the `Server (TF)` project and set the `Command` to `../game/srcds.exe`. The suggested server launch options are `-game tf -console -nomaster -insecure +sv_pure 0 +maxplayers 32 +sv_lan 1 -dev -allowdebug`.

NOTE: Team Comtress 2 is not compatible with mastercomfig. Please do not use mastercomfig or any other TF2 config.

See [the Valve Developer Wiki](https://developer.valvesoftware.com/wiki/Installing_and_Debugging_the_Source_Code) for another explanation of the last two steps.

Other launch options to consider:
- `sw` to force windowed mode
- `-w WIDTH -h HEIGHT` to set the resolution
- `+map MAPNAME` to automatically launch a map on startup

## Legal

Valve, the Valve logo, Steam, the Steam logo, Team Fortress, the Team Fortress logo, Source, the Source logo are trademarks and/or registered trademarks of Valve Corporation in the U.S. and/or other countries.

Team Comtress 2 is not sponsored, endorsed, licensed by, or affiliated with Valve Corporation.

See license for details.
