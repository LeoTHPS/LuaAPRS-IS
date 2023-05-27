#pragma once
#include <AL/Common.hpp>

struct mutex;

mutex* mutex_get_default_instance();

mutex* mutex_init();
void   mutex_deinit(mutex* mutex);

void   mutex_lock(mutex* mutex);
void   mutex_unlock(mutex* mutex);
