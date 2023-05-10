RM           ?= rm -f
CXX          ?= g++

SOURCE_FILES = main.cpp API.cpp
OBJECT_FILES = $(SOURCE_FILES:.cpp=.o)

CPPFLAGS     += 
CXXFLAGS     += -Wall -Wfatal-errors -g -std=c++20 -I. -I"$(AL_INCLUDE)"

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

		LDLIBS += -lpthread -llua5.4 -lsqlite3
	endif

	ifeq ($(PLATFORM), WINDOWS)
		LDLIBS += -luser32 -lntdll -lkernel32 -lAdvapi32
		LDLIBS += -lshlwapi
		LDLIBS += -lwsock32 -lws2_32 -lIphlpapi
		LDLIBS += -llua -lsqlite3
	endif
endif

.PHONY: clean

LuaAPRS-IS: $(OBJECT_FILES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	$(RM) $(OBJECT_FILES)
