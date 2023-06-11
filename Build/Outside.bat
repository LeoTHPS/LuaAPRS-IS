@echo off

:run_once
LuaAPRS-IS.exe Outside.lua

if %errorlevel% == 0 (
	goto run_once
)

if %errorlevel% == 65537 (
	timeout /t 5 /nobreak
	goto run_once
)

pause
