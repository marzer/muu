@ECHO off
SETLOCAL enableextensions enabledelayedexpansion
PUSHD .
CD /d "%~dp0"

REM --------------------------------------------------------------------------------------
REM 	updates all the muu-based 'singles'
REM --------------------------------------------------------------------------------------

CALL :MakeSingles ^
	type_list ^
	tagged_ptr ^
	sync_utils

POPD
@ENDLOCAL
EXIT /B 0

:MakeSingles
(
	FOR %%i IN (%*) DO (
		IF EXIST "..\..\%%~i\include\mz\%%~i.hpp.in" (
			py make_single.py "..\..\%%~i\include\mz\%%~i.hpp.in"
		)
	)
	EXIT /B
)
