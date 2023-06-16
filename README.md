# LuaAPRS-IS
###### An APRS-IS client with a Lua 5.4 API.

<hr />

### Dependencies

- [Lua 5.4](https://github.com/lua/lua)
- [libAPRS-IS](https://github.com/LeoTHPS/libAPRS-IS)
- [AbstractionLayer](https://github.com/LeoTHPS/AbstractionLayer)

### Extensions + Dependencies (Third Party)

|                                                 | [json](//github.com/nlohmann/json) | [SQLite3](//github.com/sqlite/sqlite) | [OpenSSL](//github.com/openssl/openssl) | [Discord RPC](//github.com/discord/discord-rpc) |
| ----------------------------------------------- | :--------------------------------: | :-----------------------------------: | :-------------------------------------: | :---------------------------------------------: |
| [Console](LuaAPRS-IS/Extensions/Console/)       |                                    |                                       |                                         |                                                 |
| [Discord](LuaAPRS-IS/Extensions/Discord/)       |                                    |                                       |                                         | *                                               |
| [Loop](LuaAPRS-IS/Extensions/Loop/)             |                                    |                                       |                                         |                                                 |
| [Mutex](LuaAPRS-IS/Extensions/Mutex/)           |                                    |                                       |                                         |                                                 |
| [N2YO](LuaAPRS-IS/Extensions/N2YO/)             | *                                  |                                       | *                                       |                                                 |
| [Process](LuaAPRS-IS/Extensions/Process/)       |                                    |                                       |                                         |                                                 |
| [Script](LuaAPRS-IS/Extensions/)                |                                    |                                       |                                         |                                                 |
| [SQLite3](LuaAPRS-IS/Extensions/SQLite3)        |                                    | *                                     |                                         |                                                 |
| [System](LuaAPRS-IS/Extensions/System/)         |                                    |                                       |                                         |                                                 |
| [Thread](LuaAPRS-IS/Extensions/Thread/)         |                                    |                                       |                                         |                                                 |
| [Types](LuaAPRS-IS/Extensions/Types/)           |                                    |                                       |                                         |                                                 |
| [WebRequest](LuaAPRS-IS/Extensions/WebRequest/) |                                    |                                       | *                                       |                                                 |

<hr />

### Plugins + Dependencies (Extensions)

|                                              | Console | Discord | Loop | Mutex | N2YO | Process | Script | SQLite3 | System | Thread | Types | WebRequest |
| -------------------------------------------- | :-----: | :-----: | :--: | :---: | :--: | :-----: | :----: | :-----: | :----: | :----: | :---: | :--------: |
| [Gateway](Build/Plugins/Gateway.lua)         | *       |         | *    | *     |      |         | *      | *       | *      |        |       |            |
| [Outside](Build/Plugins/Outside.lua)         | *       | *       | *    | *     |      |         | *      | *       | *      |        |       |            |
| [SkipMonitor](Build/Plugins/SkipMonitor.lua) | *       |         | *    | *     |      |         | *      | *       | *      |        |       |            |

<hr />

### Quick Start Guide

#### Install dependencies on Debian

```
sudo apt install git nlohmann-json3-dev liblua5.4-dev libsqlite3-dev libssl-dev

git clone https://github.com/LeoTHPS/libAPRS-IS libAPRS-IS
export LIB_APRS_IS_INCLUDE=/path/to/libAPRS-IS

git clone https://github.com/LeoTHPS/AbstractionLayer AbstractionLayer
export AL_INCLUDE=/path/to/AbstractionLayer
```

#### Build+Install from source

```
git clone https://github.com/LeoTHPS/LuaAPRS-IS LuaAPRS-IS
make -C LuaAPRS-IS -e COMPILER=GNU PLATFORM=LINUX
make -C LuaAPRS-IS -e COMPILER=GNU PLATFORM=LINUX install
```

<hr />

### Quick Start Script (Debian)

```
#!/bin/bash

sudo apt install git nlohmann-json3-dev liblua5.4-dev libsqlite3-dev libssl-dev

git clone https://github.com/LeoTHPS/libAPRS-IS libAPRS-IS
export LIB_APRS_IS_INCLUDE=../../../../libAPRS-IS

git clone https://github.com/LeoTHPS/AbstractionLayer AbstractionLayer
export AL_INCLUDE=../../../../AbstractionLayer

git clone https://github.com/LeoTHPS/LuaAPRS-IS LuaAPRS-IS
make -C LuaAPRS-IS/LuaAPRS-IS -e COMPILER=GNU PLATFORM=LINUX
make -C LuaAPRS-IS/LuaAPRS-IS -e COMPILER=GNU PLATFORM=LINUX install
```
