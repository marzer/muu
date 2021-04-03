@ECHO off

REM --------------------------------------------------------------------------------------
REM 	finds the Visual Studio installation on the machine and sets up the vs env
REM --------------------------------------------------------------------------------------

                        SET "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
IF NOT EXIST "%VCVARS%" SET "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
IF NOT EXIST "%VCVARS%" SET "VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
IF NOT EXIST "%VCVARS%" (
	ECHO ERROR: Could not find Visual Studio 2019
	EXIT /B 1
)

CALL "%VCVARS%"
EXIT /B %ERRORLEVEL%
