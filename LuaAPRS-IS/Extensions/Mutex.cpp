#include "Mutex.hpp"

#include <AL/OS/Mutex.hpp>

struct mutex
{
	AL::OS::Mutex mutex;
};

mutex  mutex_default_instance;

mutex* mutex_get_default_instance()
{
	return &mutex_default_instance;
}

mutex* mutex_init()
{
	auto mutex = new ::mutex
	{
	};

	return mutex;
}
void   mutex_deinit(mutex* mutex)
{
	if ((mutex != nullptr) && (mutex != &mutex_default_instance))
		delete mutex;
}

void   mutex_lock(mutex* mutex)
{
	if (mutex != nullptr)
		mutex->mutex.Lock();
}
void   mutex_unlock(mutex* mutex)
{
	if (mutex != nullptr)
		mutex->mutex.Unlock();
}
