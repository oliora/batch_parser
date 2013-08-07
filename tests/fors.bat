@echo off
rem FOR %I IN (set) DO command
rem FOR /D %I IN (set) DO command

FOR /F "usebackq delims==" %%i IN (`set`) DO @echo %%i

FOR /R . %%i IN (*.bat) DO echo %%i

FOR===/L===%%I=IN====(====1====,===1===,====5====)====DO=echo %%I

FOR /L %%I IN (1, 1, 5) DO (
echo %%I
)

rem FOR /F ["options"] %variable IN (file-set) DO command

FOR /F %%i IN ('echo ")foo"') DO echo %%i