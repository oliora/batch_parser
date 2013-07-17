@echo off
echo ^^
echo ^a
echo b^   

echo c^ & echo d
echo ^ & echo e
echo ^& echo f
echo ^" & echo g


call :print 1^ 2

goto :EOF

:print
echo %%1=%1, %%2=%2