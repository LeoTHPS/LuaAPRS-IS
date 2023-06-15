CP           ?= cp -f
RM           ?= rm -f
CXX          ?= g++

SOURCE_FILES  = main.cpp
OBJECT_FILES  = $(SOURCE_FILES:.cpp=.o)

LDFLAGS      += -shared
CPPFLAGS     += 
CXXFLAGS     += -Wall -Wfatal-errors $(OPTIMIZATION_FLAGS) -std=c++20 -fPIC -I. -I"$(AL_INCLUDE)" -I"$(LIB_APRS_IS_INCLUDE)" -I"../../"

ifdef COMPILER
	ifeq ($(COMPILER), GNU)
		CXX = g++
	else ifeq ($(COMPILER), CLANG)
		CXX = clang++
	else
		CXX = $(COMPILER)
	endif
endif

ifdef PLATFORM
	ifeq ($(PLATFORM), LINUX)
#		liblua5.4-dev libsqlite3-dev

		LDLIBS += -lpthread -llua5.4
	endif

	ifeq ($(PLATFORM), WINDOWS)
		LDLIBS += -luser32 -lkernel32 -lshlwapi
		LDLIBS += -lGdi32
		LDLIBS += -llua
	endif
endif

.PHONY: clean install uninstall

Discord$(FILE_EXTENSION): $(OBJECT_FILES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	$(RM) $(OBJECT_FILES)

install:
	$(CP) discord-rpc.dll $(RELEASE_PATH)/discord-rpc.dll
	$(CP) Discord.lua $(RELEASE_EXTENSION_PATH)/Discord.lua
	$(CP) Discord$(FILE_EXTENSION) $(RELEASE_EXTENSION_PATH)/Discord$(FILE_EXTENSION)

uninstall:
	$(RM) $(RELEASE_PATH)/discord-rpc.dll
	$(RM) $(RELEASE_EXTENSION_PATH)/Discord.lua
	$(RM) $(RELEASE_EXTENSION_PATH)/Discord$(FILE_EXTENSION)
