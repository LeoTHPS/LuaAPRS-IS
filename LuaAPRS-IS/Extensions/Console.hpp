#pragma once
#include <AL/Common.hpp>

bool        console_set_title(const char* value);

char        console_read();
const char* console_read_line();

bool        console_write(const char* value);
bool        console_write_line(const char* value);

bool        console_enable_quick_edit_mode();
bool        console_disable_quick_edit_mode();
