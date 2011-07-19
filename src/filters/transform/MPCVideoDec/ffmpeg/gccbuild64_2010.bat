@ECHO OFF
SETLOCAL

rem Check for the help switches
IF /I "%~1"=="help"   GOTO SHOWHELP
IF /I "%~1"=="/help"  GOTO SHOWHELP
IF /I "%~1"=="-help"  GOTO SHOWHELP
IF /I "%~1"=="--help" GOTO SHOWHELP
IF /I "%~1"=="/?"     GOTO SHOWHELP

IF DEFINED MINGW64 GOTO VarOk
ECHO ERROR: Please define MINGW64 (and/or MSYS) environment variable(s)
ENDLOCAL
EXIT /B

:VarOk
SET PATH=%MSYS%\bin;%MINGW64%\bin;%PATH%

IF /I "%~2" == "Debug" SET "DEBUG=DEBUG=yes"

IF "%~1" == "" (
  SET "BUILDTYPE=build"
  CALL :SubMake
  EXIT /B
) ELSE (
  IF /I "%~1" == "Build" (
    SET "BUILDTYPE=build"
    CALL :SubMake
    EXIT /B
  )

  IF /I "%~1" == "Clean" (
    SET "BUILDTYPE=clean"
    CALL :SubMake clean
    EXIT /B
  )

  IF /I "%~1" == "Rebuild" (
    SET "BUILDTYPE=rebuild"
    CALL :SubMake clean
    CALL :SubMake
    EXIT /B
  )

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  ENDLOCAL
  EXIT /B
)


:SubMake
IF /I "%1" == "Clean" (SET "JOBS=-j1") ELSE (SET "JOBS=-j4")

TITLE "make %JOBS% 64BIT=yes %DEBUG% %*"
ECHO make %JOBS% 64BIT=yes %DEBUG% %*
make.exe %JOBS% 64BIT=yes %DEBUG% %*

ENDLOCAL
EXIT /B


:SHOWHELP
TITLE "%~nx0 %1"
ECHO. & ECHO.
ECHO Usage:   %~nx0 [Clean^|Build^|Rebuild] [Debug]
ECHO.
ECHO Notes:   The arguments are not case sensitive.
ECHO. & ECHO.
ECHO Executing "%~nx0" will use the defaults: "%~nx0 build"
ECHO.
ENDLOCAL
EXIT /B
