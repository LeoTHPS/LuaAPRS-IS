@echo off

set EXIT_CODE_SUCCESS=0
set EXIT_CODE_USER_DEFINED=1000
set SCRIPT_EXIT_CODE_APRS_IS_CONNECTION_CLOSED=1
set SCRIPT_EXIT_CODE_APRS_IS_CONNECTION_FAILED=2
set SCRIPT_EXIT_CODE_SQLITE3_OPEN_FAILED=3
set SCRIPT_EXIT_CODE_STORAGE_LOAD_FAILED=%EXIT_CODE_USER_DEFINED%+1

:run_once
LuaAPRS-IS.exe Outside.lua

if %errorlevel% == %EXIT_CODE_SUCCESS% (
	goto run_once
)

echo %errorlevel%

if %errorlevel% == %SCRIPT_EXIT_CODE_APRS_IS_CONNECTION_CLOSED% (
	timeout /t 3 /nobreak
	goto run_once
)

if %errorlevel% == %SCRIPT_EXIT_CODE_APRS_IS_CONNECTION_FAILED% (
	timeout /t 5 /nobreak
	goto run_once
)

pause
