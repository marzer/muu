@ECHO off
SETLOCAL enableextensions enabledelayedexpansion
PUSHD .
CD /d "%~dp0\.."

REM --------------------------------------------------------------------------------------
REM 	Refreshes local submodules with their remotes from git
REM --------------------------------------------------------------------------------------

CALL :DeleteDirectories ^
	external\Catch2

git config http.postBuffer 33554432
git config protocol.version 2
git config diff.ignoreSubmodules dirty
git submodule update --init --depth 1 --jobs 4

POPD
@ENDLOCAL
EXIT /B

:DeleteDirectories
(
	FOR %%i IN (%*) DO (
		IF EXIST "%%~i" (
			ECHO Deleting "%%~i"
			RMDIR /S /Q "%%~i"
		)
	)
	EXIT /B
)
