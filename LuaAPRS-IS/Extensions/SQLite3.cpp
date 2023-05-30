#include "SQLite3.hpp"

#include <AL/OS/Console.hpp>

struct _sqlite3
{
	AL::SQLite3::Database database;
};

struct _sqlite3_query_result_row
{
	AL::SQLite3::DatabaseQueryResultRow* row;
};

_sqlite3*   _sqlite3_init(const char* database_path, SQLITE3_DATABASE_FLAGS flags)
{
	auto _sqlite3 = new ::_sqlite3
	{
		.database = AL::SQLite3::Database(AL::FileSystem::Path(database_path), static_cast<AL::SQLite3::DatabaseFlags>(flags))
	};

	return _sqlite3;
}
void        _sqlite3_deinit(_sqlite3* sqlite3)
{
	if (sqlite3 != nullptr)
	{
		if (_sqlite3_is_open(sqlite3))
			_sqlite3_close(sqlite3);

		delete sqlite3;
	}
}

bool        _sqlite3_is_open(_sqlite3* sqlite3)
{
	return sqlite3 && sqlite3->database.IsOpen();
}

bool        _sqlite3_open(_sqlite3* sqlite3)
{
	if (_sqlite3_is_open(sqlite3))
		return false;

	try
	{
		sqlite3->database.Open();
	}
	catch (const AL::Exception& exception)
	{
		AL::OS::Console::WriteException(
			exception
		);

		return false;
	}

	return true;
}
void        _sqlite3_close(_sqlite3* sqlite3)
{
	if (_sqlite3_is_open(sqlite3))
	{
		sqlite3->database.Close();
	}
}

bool        _sqlite3_execute_query(_sqlite3* sqlite3, const char* query, _sqlite3_query_callback callback)
{
	if (!_sqlite3_is_open(sqlite3))
		return false;

	AL::SQLite3::DatabaseQueryResult result;

	try
	{
		result = sqlite3->database.Query(query);
	}
	catch (const AL::Exception& exception)
	{
		AL::OS::Console::WriteException(
			exception
		);

		return false;
	}

	_sqlite3_query_result_row query_result_row;
	AL::uint32                query_result_row_index = 0;

	for (auto& result_row : result)
	{
		query_result_row.row = &result_row;

		callback(sqlite3, &query_result_row, query_result_row_index++);
	}

	return true;
}
bool        _sqlite3_execute_non_query(_sqlite3* sqlite3, const char* query)
{
	if (!_sqlite3_is_open(sqlite3))
		return false;

	try
	{
		sqlite3->database.Query(query);
	}
	catch (const AL::Exception& exception)
	{
		AL::OS::Console::WriteException(
			exception
		);

		return false;
	}

	return true;
}

AL::uint32  _sqlite3_query_result_row_get_column_count(_sqlite3_query_result_row* query_result_row)
{
	return static_cast<AL::uint32>(query_result_row ? query_result_row->row->Columns.GetSize() : 0);
}
const char* _sqlite3_query_result_row_get_column_name(_sqlite3_query_result_row* query_result_row, AL::uint32 index)
{
	return (query_result_row && (index < query_result_row->row->Columns.GetSize())) ? query_result_row->row->Columns[index].GetCString() : nullptr;
}
const char* _sqlite3_query_result_row_get_column_value(_sqlite3_query_result_row* query_result_row, AL::uint32 index)
{
	return (query_result_row && (index < query_result_row->row->Values.GetSize())) ? query_result_row->row->Values[index].GetCString() : nullptr;
}
