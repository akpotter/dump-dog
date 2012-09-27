@echo off

echo ---=== Performing Dump Dog Cleanup ... ===---

cd /d .\dumps

for %%X in (.\*.dmp) do (
echo processing [%%X]
..\7z a -tzip "%%X.zip" "%%X"
echo deleting %%X
del "%%X"
echo copying %%X
copy "%%X.zip" \\crash\data
ECHO DONE > "\\crash\data\%%X.done"
)

if ERRORLEVEL 1 goto error

del .\*.zip

if ERRORLEVEL 1 goto error
cd /d ..
echo Done!
goto end

:error
echo Error occured. Stopping the DumpDog cleanup process.
pause

:end

