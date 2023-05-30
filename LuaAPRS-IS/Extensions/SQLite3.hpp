#pragma once
#include <AL/Common.hpp>

#include <AL/Lua54/Lua.hpp>

#include <AL/SQLite3/Database.hpp>

struct _sqlite3;
struct _sqlite3_query_result_row;

typedef typename AL::Get_Enum_Or_Integer_Base<AL::SQLite3::DatabaseFlags>::Type SQLITE3_DATABASE_FLAG;

enum SQLITE3_DATABASE_FLAGS : SQLITE3_DATABASE_FLAG
{
	SQLITE3_DATABASE_FLAG_NONE          = static_cast<SQLITE3_DATABASE_FLAG>(AL::SQLite3::DatabaseFlags::None),

	SQLITE3_DATABASE_FLAG_URI           = static_cast<SQLITE3_DATABASE_FLAG>(AL::SQLite3::DatabaseFlags::URI),
	SQLITE3_DATABASE_FLAG_CREATE        = static_cast<SQLITE3_DATABASE_FLAG>(AL::SQLite3::DatabaseFlags::Create),
	SQLITE3_DATABASE_FLAG_READ_ONLY     = static_cast<SQLITE3_DATABASE_FLAG>(AL::SQLite3::DatabaseFlags::ReadOnly),
	SQLITE3_DATABASE_FLAG_READ_WRITE    = static_cast<SQLITE3_DATABASE_FLAG>(AL::SQLite3::DatabaseFlags::ReadWrite),
	SQLITE3_DATABASE_FLAG_MEMORY        = static_cast<SQLITE3_DATABASE_FLAG>(AL::SQLite3::DatabaseFlags::Memory),

	SQLITE3_DATABASE_FLAG_NO_MUTEX      = static_cast<SQLITE3_DATABASE_FLAG>(AL::SQLite3::DatabaseFlags::NoMutex),
	SQLITE3_DATABASE_FLAG_FULL_MUTEX    = static_cast<SQLITE3_DATABASE_FLAG>(AL::SQLite3::DatabaseFlags::FullMutex),

	SQLITE3_DATABASE_FLAG_NO_FOLLOW     = static_cast<SQLITE3_DATABASE_FLAG>(AL::SQLite3::DatabaseFlags::NoFollow),

	SQLITE3_DATABASE_FLAG_SHARED_CACHE  = static_cast<SQLITE3_DATABASE_FLAG>(AL::SQLite3::DatabaseFlags::SharedCache),
	SQLITE3_DATABASE_FLAG_PRIVATE_CACHE = static_cast<SQLITE3_DATABASE_FLAG>(AL::SQLite3::DatabaseFlags::PrivateCache)
};

typedef AL::Lua54::Function::LuaCallback<void(_sqlite3* sqlite3, _sqlite3_query_result_row* query_result_row, AL::uint32 query_result_row_index)> _sqlite3_query_callback;

_sqlite3*   _sqlite3_init(const char* database, SQLITE3_DATABASE_FLAGS flags);
void        _sqlite3_deinit(_sqlite3* sqlite3);

bool        _sqlite3_is_open(_sqlite3* sqlite3);

bool        _sqlite3_open(_sqlite3* sqlite3);
void        _sqlite3_close(_sqlite3* sqlite3);

bool        _sqlite3_execute_query(_sqlite3* sqlite3, const char* query, _sqlite3_query_callback callback);
bool        _sqlite3_execute_non_query(_sqlite3* sqlite3, const char* query);

AL::uint32  _sqlite3_query_result_row_get_column_count(_sqlite3_query_result_row* query_result_row);
const char* _sqlite3_query_result_row_get_column_name(_sqlite3_query_result_row* query_result_row, AL::uint32 index);
const char* _sqlite3_query_result_row_get_column_value(_sqlite3_query_result_row* query_result_row, AL::uint32 index);
