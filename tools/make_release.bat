@ECHO off
SETLOCAL enableextensions enabledelayedexpansion
PUSHD .
CD /d "%~dp0"

REM --------------------------------------------------------------------------------------
REM 	Invokes python to make a release.
REM --------------------------------------------------------------------------------------

py -OO make_release.py %*
if %ERRORLEVEL% NEQ 0 (
	PAUSE
	GOTO FINISH
)

:FINISH
POPD
@ENDLOCAL
EXIT /B %ERRORLEVEL%
