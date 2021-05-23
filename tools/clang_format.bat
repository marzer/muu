@ECHO off
SETLOCAL enableextensions enabledelayedexpansion
PUSHD .
CD /d "%~dp0\.."

REM --------------------------------------------------------------------------------------
REM 	Runs clang format on all the non-test C++ files in the project
REM --------------------------------------------------------------------------------------

CALL :RunClangFormatOnDirectories ^
	src ^
	include\muu ^
	examples

GOTO FINISH

:RunClangFormatOnDirectories
(
	FOR %%i IN (%*) DO (
		IF EXIST "%%~i" (
			ECHO Formatting files in "%%~i"
			clang-format --style=file -i "%%~i\*.cpp" >nul 2>&1
			clang-format --style=file -i "%%~i\*.h" >nul 2>&1
		)
	)
	EXIT /B
)

:FINISH
POPD
@ENDLOCAL
EXIT /B 0
