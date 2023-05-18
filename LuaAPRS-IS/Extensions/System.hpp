#pragma once
#include <AL/Common.hpp>

void       system_sleep(AL::uint32 ms);

AL::uint64 system_get_idle_time();

AL::uint64 system_get_timestamp();
