@ECHO off
SETLOCAL enableextensions enabledelayedexpansion
PUSHD .
CD /d "%~dp0"

REM --------------------------------------------------------------------------------------
REM 	updates all the muu-based 'singles'
REM --------------------------------------------------------------------------------------

py make_single.py ..\..\type_list\include\mz\type_list.hpp.in
py make_single.py ..\..\tagged_ptr\include\mz\tagged_ptr.hpp.in

EXIT /B %ERRORLEVEL%
