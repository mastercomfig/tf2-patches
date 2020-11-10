[![ko-fi](https://img.shields.io/badge/Support%20me%20on-Ko--fi-FF5E5B.svg?logo=ko-fi&style=flat-square)](https://ko-fi.com/mastercoms)
[![Liberapay](https://img.shields.io/liberapay/receives/mastercoms.svg?logo=liberapay&style=flat-square)](https://liberapay.com/mastercoms/)
[![Steam donate](https://img.shields.io/badge/Donate%20via-Steam-00adee.svg?style=flat-square&logo=steam)](https://steamcommunity.com/tradeoffer/new/?partner=85845165&token=M9cQHh8N)
[![Join the Discord chat](https://img.shields.io/badge/Discord-%23comtress--client-7289da.svg?style=flat-square&logo=discord)](https://discord.gg/CuPb2zV)

# Welcome to Team Comtress 2!

## What *is* TC2?

Team Comtress 2 is a fork of an older version of Team Fortress 2, with a community development team fixing bugs, improving performance, and adding quality of life features, with the goal of having those changes pulled upstream by Valve to the modern game.

## What is TC2 *not*?

* **TC2 is *not* a "pro mod"**

While much of the work in TC2 will improve the competitive experience, TC2 does not touch gameplay, balance, or competitive rules.

* **TC2 is *not* a vision for the "good old days" of TF2**

TC2 will not feature game design changes, gameplay additions, or almost any form of new content. If you're looking for this kind of content, [Team Fortress 2 Classic](https://tf2classic.com/) is more up your alley.

* **TC2 is *not* 2007-2008 TF2**

You're looking for https://github.com/NicknineTheEagle/TF2-Base

* **TC2 is *not* the modern game, with new content/updates**

You're looking for [Creators.TF](https://creators.tf/).

* **TC2 is *not* an alternative, more stable game experience**

TC2 is based on the 2017 TF2 source code leak, which contained an in-development version of Jungle Inferno. This build is rife with issues and incomplete features, and ***is not compatible*** with the modern game client.

## How can I help?

1. **Report bugs that exist in live TF2 *or* TC2 through the [issues page](https://github.com/mastercomfig/team-comtress-2/issues).**
	* Bugs that are present in modern TF2 but are not present in TC2 cannot be fixed in this project.
2. **Suggest quality of life improvements (*not balance changes*) through the [issues page](https://github.com/mastercomfig/team-comtress-2/issues).**
	* These changes should not affect gameplay, and should be unanimously desired.
3. **Install TC2 and play the game to test stability, bugfixes, and performance.**
	* As a separate game, you cannot play normal TF2 matchmaking. You can *only* connect to TC2 servers.
	* Information about multiplayer playtests can be found on our [Discord server](https://discord.gg/CuPb2zV).

## Basic Installation

New users should follow this approach.
<!--### Automatic

https://github.com/GoodOldJack12/Team-Comtress-2-Installer#Install-guide

### Manual
-->
1. Go to your Steam Library, right click Team Fortress 2, click `Properties...`, go to the `Updates` tab, and untick `Enable Steam Cloud Synchronization`.
2. Go to the `Local Files` tab, and click `Browse Local Files...`
3. Copy `Team Fortress 2` to a new folder.
4. Delete the following in the `Team Fortress 2` **COPY** folder:
	* `tf/`
		* `addons`
		* `custom`
		* `cfg/`
			* `autoexec.cfg`
			* `config.cfg`
			* `listenserver.cfg`
			* `scout.cfg`
			* `soldier.cfg`
			* `pyro.cfg`
			* `demoman.cfg`
			* `heavyweapons.cfg`
			* `engineer.cfg`
			* `medic.cfg`
			* `sniper.cfg`
			* `spy.cfg`
5. Download `game_clean.zip` from [the latest release](https://github.com/mastercomfig/team-comtress-2/releases/latest).
6. Right click `game_clean.zip`, click `Properties`, tick `Unblock`, and press OK:
	* Note: The file extension (`.zip`) might not be visible.
	<br>![](https://support.winzip.com/hc/article_attachments/360059191533/unblock3.png)
	<br>If there is no option to Unblock it, you can skip this step.
7. Extract `game_clean.zip` into your `Team Fortress 2` copy overwriting all files.
8. To run TC2, open `start_tf2.bat`.
	* Note: Steam must be running.

## For players:

* [Things to keep in mind with TC2](https://github.com/mastercomfig/team-comtress-2/wiki/Things-to-keep-in-mind-with-TC2)
* [New console commands and launch options](https://github.com/mastercomfig/team-comtress-2/wiki/New-console-commands-and-launch-options)
* [Edited and removed console commands](https://github.com/mastercomfig/team-comtress-2/wiki/Edited-and-removed-console-commands)

## For developers:

### Advanced Installation

Advanced Installation instructions for contributors and developers can be found [here](https://github.com/mastercomfig/team-comtress-2/wiki/Advanced-Installation-Instructions).

### Build Instructions:

* [Windows](https://github.com/mastercomfig/team-comtress-2/wiki/Windows-Build-Instructions)
* [Linux](https://github.com/mastercomfig/team-comtress-2/wiki/Linux-Build-Instructions)
* ~~[Mac](https://github.com/mastercomfig/team-comtress-2/wiki/Mac-Build-Instructions)~~

## Legal

Valve, the Valve logo, Steam, the Steam logo, Team Fortress, the Team Fortress logo, Source, the Source logo are trademarks and/or registered trademarks of Valve Corporation in the U.S. and/or other countries.

Team Comtress 2 is not sponsored, endorsed, licensed by, or affiliated with Valve Corporation.

See license for details.
