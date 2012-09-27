@echo off
CLS
:start
echo ---=== Dump Dog Installation ===---
ECHO Select Drive to install DumpDog.
ECHO Dumps will be stored in the drive
ECHO Choose the drive with the most available space
ECHO For example press 'D' for D:\

SET Choice=
SET /P Choice=Type the letter and press Enter: 

set NameCheck=%Choice:~0,1%
 if not "%NameCheck%"=="%Choice%" (
    cls
    echo ------------==================------------
    echo ERROR: Drive Input cannot be more than 1 char !!!
    echo ------------==================------------
    ECHO.
   goto start
  )


IF NOT EXIST C:\dumpdog md c:\dumpdog\
ECHO %Choice%:/ > c:\dumpdog\dumpdog_drive.txt




IF NOT EXIST %Choice%:\dumpdog md %Choice%:\dumpdog\
IF NOT EXIST %Choice%:\dumpdog\dumps md %Choice%:\dumpdog\dumps

copy "\\crash\data\Client-Install\DumpDog Cleanup.bat" %Choice%:\dumpdog\
copy "\\crash\data\Client-Install\Gamer Shutdown.bat" %Choice%:\dumpdog\




copy \\crash\data\Client-Install\l_WatchDog.exe c:\dumpdog\l_watchdog.exe
copy "\\crash\data\Client-Install\*.wav" c:\dumpdog\
::copy \\crash\data\Client-Install\compressor.exe %Choice%:\dumpdog\compressor.exe
copy \\crash\data\Client-Install\7z.* %Choice%:\dumpdog\




if ERRORLEVEL 1 goto error

echo.
echo ------------==================------------
echo Done! DumpDog is installed on drive %Choice%.
echo ------------==================------------
pause
goto end

:error
echo Error occured. Stopping the build process...
@pause

:end