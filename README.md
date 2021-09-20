# Fugly changes

- Enabled sound
- Enabled basic user input
- Null AI to not interfere with user input
- license is now GPL 3, sorry proprietary bots
- Partially replaced cmake with gnu make

# BWAPI for OpenBW

This is a fork of BWAPI which allows the use of OpenBW as a backend for BWAPI.
The BWAPI version is 4.2.0.

This fork has significant changes and it no longer works with regular StarCraft: Brood War. Support for OpenBW should eventually be merged into the official BWAPI, but this fork will exist until then.

This is a development version, and breaking changes can occur at any time (that goes for OpenBW itself too).

### build & install

TODO.

The UI can be disabled either by setting environment variable OPENBW_ENABLE_UI=0 or calling Broodwar->setGUI(false).

## Compiling/linking your bot

If you installed, BWAPI.h (and the rest) will be in the include folder, otherwise they're found in bwapi/bwapi/include in the source tree.

On Windows, you should link to BWAPILIB.lib (not BWAPI.lib, this is different from regular BWAPI).

On Linux, you don't really need to link to anything, but you can -lBWAPILIB if you want.

### Running

The main executable is BWAPILauncher. Running BWAPILauncher is the functional equivalent of starting StarCraft & BWAPI via an injector on Windows.

OpenBW needs the usual 3 mpq files in the working directory when it starts (Stardat.mpq, Broodat.mpq and Patch_rt.mpq).
These can be copied from StarCraft: BroodWar 1.16.1 or 1.18, or you can just start BWAPILauncher from the StarCraft directory.

BWAPI loads its configuration from bwapi-data/bwapi.ini as usual. See https://github.com/bwapi/bwapi/wiki/Configuration.

the image below outlines the directory structure and required files to run OpenBW:

![directory structure](https://i.imgur.com/c2nE4Go.png)

OpenBW has no built-in computer player, so in single player games the opponent does not peform any actions.

Keep in mind that BWAPI configuration settings can be set through environment variables, so one can for instance run
```
BWAPI_CONFIG_AI__AI=/path/to/my/bot.so BWAPI_CONFIG_AUTO_MENU__RACE=Zerg BWAPILauncher
```

In general BWAPILauncher has no console output unless there is an error.

### Multiplayer

Multiplayer games are supported, but playing together with StarCraft: Brood War clients is currently not possible.

Only 1v1 games are supported. More than two clients can not connect together.

The map (in bwapi.ini) must be the same on both clients, otherwise each player will load a different map and things will break.

Multiplayer is enabled as usual by setting auto_menu=LAN in bwapi.ini (or through environment variable), but the lan_mode setting is not used.
Instead, the OPENBW_LAN_MODE environment variable is read.

The possible settings for `OPENBW_LAN_MODE` is `TCP`, `LOCAL`, `LOCAL_AUTO`, `FILE` or `FD`. On Windows only TCP is supported, though :)

The default is LOCAL_AUTO (except on Windows, where it is TCP)


The variables mentioned below are environment variables.
#### TCP
Uses TCP/IP for networking. By default it will bind to 0.0.0.0 and listen on port 6112, and connect to 127.0.0.1 on the same port.

`OPENBW_TCP_LISTEN_HOSTNAME` and `OPENBW_TCP_LISTEN_PORT` can be set to change the interface and port that it listens on.
`OPENBW_TCP_CONNECT_HOSTNAME` and `OPENBW_TCP_CONNECT_PORT` can be set to change the hostname and port that it tries to connect to.

There are no errors if it fails to bind or connect.

#### LOCAL
Uses local sockets for interprocess communication.

`OPENBW_LOCAL_PATH` - the path to the socket file to use. If it exists, then it attempts to connect, otherwise it will create the file and listen on it.
One of the clients has to listen, so when starting two clients, the file should not already exist.

#### LOCAL_AUTO
Listens on a unique socket file in /tmp/openbw and tries to connect to other sockets in that directory. It will automatically remove any unused sockets.

`OPENBW_LOCAL_AUTO_DIRECTORY` can be used to change the directory that it uses.

#### FILE
`OPENBW_FILE_READ` is the file to read from, and `OPENBW_FILE_WRITE` is the file to write to. The files should be pipe (FIFO) files or something like that.

#### FD
Same as FILE, but `OPENBW_FD_READ` and `OPENBW_FD_WRITE` can be used to specify file descriptors instead of file paths.

