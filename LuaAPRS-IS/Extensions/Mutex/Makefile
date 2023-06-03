CP           ?= cp -f
RM           ?= rm -f
CXX          ?= g++

SOURCE_FILES  = main.cpp
OBJECT_FILES  = $(SOURCE_FILES:.cpp=.o)

LDFLAGS      += -static -shared
CPPFLAGS     += 
CXXFLAGS     += -Wall -Wfatal-errors -g -std=c++20 -I. -I"$(AL_INCLUDE)" -I"$(LIB_APRS_IS_INCLUDE)" -I"../../"

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
#		liblua5.4-dev

		LDLIBS += -llua5.4
	endif

	ifeq ($(PLATFORM), WINDOWS)
		LDLIBS += -luser32 -lntdll -lkernel32 -lAdvapi32 -lshlwapi
		LDLIBS += -llua
	endif
endif

.PHONY: clean install uninstall

Mutex$(SCRIPT_EXTENSION_EXT): $(OBJECT_FILES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	$(RM) $(OBJECT_FILES)

install:
	$(CP) Mutex.lua $(SCRIPT_EXTENSION_PATH)/Mutex.lua
	$(CP) Mutex$(SCRIPT_EXTENSION_EXT) $(SCRIPT_EXTENSION_PATH)/Mutex$(SCRIPT_EXTENSION_EXT)

uninstall:
	$(RM) $(SCRIPT_EXTENSION_PATH)/Mutex.lua
	$(RM) $(SCRIPT_EXTENSION_PATH)/Mutex$(SCRIPT_EXTENSION_EXT)
