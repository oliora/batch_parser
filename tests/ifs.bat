@echo off

rem if ERRORLEVEL
if ERRORLEVEL 0 echo foo1

rem if NOT ERRORLEVEL
if NOT errorlevel 1 echo foo2

rem if str equal
if mamba == mamba echo foo3

rem if NOT str equal
if NOT foo==bar echo foo4

rem if EXIST
if exist ifs.bat echo foo5

rem if NOT EXIST
if NOT EXIST iifs.bat echo foo6

rem if str cmp
if "mam ba" equ "mam ba" echo foo7

rem if NOT str cmp
if NOT "mam ba" NEQ "mam ba" echo foo8

rem if /I NOT str cmp. Note that "... not /I ..." is bad format
if /I not "mam ba" NEQ "mam ba" echo foo9

rem if /I NOT str cmp
if /I "mam ba" equ "mam ba" echo foo10

rem if CMDEXTVERSION
if CMDEXTVERSION 1 echo foo11

rem if NOT CMDEXTVERSION
if not cmdextversion 999 echo foo12

rem if DEFINED
if DEFINED PATH echo foo13

rem if NOT DEFINED
if not defined mamba_italiana echo foo14

rem if else
if NOT foo==bar (echo foo90) else (echo bar90)

rem if else multiline
if NOT foo==bar (
  echo foo100
) else (
  echo bar100
)

rem if with group
if ERRORLEVEL 0 (
echo foo101
echo bar101
)

rem if with caret and several commands
if ERRORLEVEL 0 ^
echo foo102 &&^
echo bar102

rem FAIL if with caret and group
if ERRORLEVEL 0 ^
 (echo foo999)
