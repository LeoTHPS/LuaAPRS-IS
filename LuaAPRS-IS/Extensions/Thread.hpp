#pragma once
#include <AL/Common.hpp>

#include <AL/Lua54/Lua.hpp>

struct thread;

typedef AL::Lua54::Function::LuaCallback<void(thread* thread)> thread_start_callback;

void    thread_sleep(AL::uint32 ms);

thread* thread_start(thread_start_callback callback);
bool    thread_join(thread* thread, AL::uint32 max_wait_time_ms);
