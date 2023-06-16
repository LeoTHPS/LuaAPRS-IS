# LuaAPRS-IS
###### An APRS-IS client with a Lua 5.4 API.

<hr />

### Dependencies
- [Lua 5.4](https://github.com/lua/lua)
- [SQLite3](https://github.com/sqlite/sqlite)
- [OpenSSL](https://github.com/openssl/openssl)
- [libAPRS-IS](https://github.com/LeoTHPS/libAPRS-IS)
- [AbstractionLayer](https://github.com/LeoTHPS/AbstractionLayer)

<hr />

### Quick Start Guide

#### Install dependencies on Debian

```
sudo apt install liblua5.4-dev libsqlite3-dev libssl-dev

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

sudo apt install git liblua5.4-dev libsqlite3-dev libssl-dev

git clone https://github.com/LeoTHPS/libAPRS-IS libAPRS-IS
export LIB_APRS_IS_INCLUDE=../../../../libAPRS-IS

git clone https://github.com/LeoTHPS/AbstractionLayer AbstractionLayer
export AL_INCLUDE=../../../../AbstractionLayer

git clone https://github.com/LeoTHPS/LuaAPRS-IS LuaAPRS-IS
make -C LuaAPRS-IS/LuaAPRS-IS -e COMPILER=GNU PLATFORM=LINUX
make -C LuaAPRS-IS/LuaAPRS-IS -e COMPILER=GNU PLATFORM=LINUX install
```

<hr />

### Notes about the Discord extension

It depends on [Discord RPC](https://github.com/discord/discord-rpc) and requires discord-rpc.dll to be in the [/LuaAPRS-IS/Extensions/Discord/](tree/master/LuaAPRS-IS/Extensions/Discord/) directory before installing LuaAPRS-IS. Since this extension will have no use to most people and currently only works on Windows I recommend simply deleting it after cloning and before building (``rm -r LuaAPRS-IS/LuaAPRS-IS/Extensions/Discord``).
