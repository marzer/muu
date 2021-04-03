@ECHO off
SETLOCAL enableextensions enabledelayedexpansion
PUSHD .
CD /d "%~dp0\.."

REM --------------------------------------------------------------------------------------
REM 	builds all the MSVC variants of the libs 
REM --------------------------------------------------------------------------------------

CALL tools\vcvars.bat
IF %ERRORLEVEL% NEQ 0 GOTO FINISH

FOR %%p IN (x64 Win32) DO (
	FOR %%c IN (Debug Release) DO (
		FOR %%t IN (DynamicLibrary StaticLibrary) DO (
			ECHO.
			ECHO --------------------------------------------------------------------------------------
			ECHO Building %%p %%c %%t %*
			ECHO --------------------------------------------------------------------------------------
			ECHO.
			MSBuild muu.vcxproj ^
				-nologo ^
				-m ^
				-t:Rebuild ^
				-p:SolutionPath="%cd%\\muu.sln" ^
				-p:Platform=%%p ^
				-p:Configuration=%%c ^
				-p:MuuConfigurationType=%%t ^
				%*
			IF !ERRORLEVEL! NEQ 0 GOTO FINISH
		)
	)
)

:FINISH
POPD
@ENDLOCAL
EXIT /B %ERRORLEVEL%
