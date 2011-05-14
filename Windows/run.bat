@echo off

REM This script starts tkeden on Windows
REM $Id: run.bat,v 1.10 2001/07/27 17:06:32 cssbz Exp $

REM This file should no longer be required as tkeden attempts some
REM guessing at runtime about this configuration.  It might be required
REM if you are attempting to set up file extensions to be associated
REM with tkeden however (as the current directory might not then be
REM the tkeden installation directory).

REM INSTALLATION CONFIGURATION REQUIRED HERE!
REM
REM Here, the directory containing tkeden.exe and its support
REM files must be specified.
REM
REM Don't install tkeden in "Program Files" or any place where
REM a space would be in the pathname.
REM
REM The directory must be specified as a full DOS pathname and
REM must not have a trailing slash: for example,
REM set INSTALL_BASE=C:\eden-1.15
REM
set INSTALL_BASE=C:\cygwin\home\ashley\tkeden1.15\Windist

REM You shouldn't need to configure anything beneath this point.
set TKEDEN_LIB=%INSTALL_BASE%\lib-tkeden
set TCL_LIBRARY=%INSTALL_BASE%\share\tcl8.0

echo TKEDEN_LIB is %TKEDEN_LIB%
echo TCL_LIBRARY is %TCL_LIBRARY%

if exist .\tkeden.exe goto DIST
if exist ..\tkeden.exe goto DEVEL

echo "Cannot find tkeden.exe"
exit
:DIST
REM tkeden is in the current directory - so this is the minimal version
.\tkeden.exe -l%TKEDEN_LIB%
goto END

:DEVEL
REM tkeden is in the directory below, so this script is probably in the
REM Windows subdirectory and is probably the development version
..\tkeden.exe -l%TKEDEN_LIB%
goto END

:END
echo tkeden has finished

