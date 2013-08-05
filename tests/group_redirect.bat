@echo off
(echo foo) >r1.out
(
echo foo) 1>r2.out
(
echo foo
) >>r3.out
(echo foo
) 1>>r4.out
>r5.out (
echo foo)
1>r6.out (
echo foo
)
>>r7.out (echo foo)
1>>r8.out (echo foo
)

rem OK:  
(echo foo) >^
outfile
rem OK:  
(echo foo) >>^
outfile
rem OK:  
(echo foo) ^
> outfile
rem Works as '(echo foo) > " outfile"':
(echo foo) ^
 > outfile
rem FAIL:
(echo foo) ^
 > outfile
rem FAIL:  
(echo foo) ^
2> outfile
rem FAIL:
(echo foo) ^
>> outfile
rem FAIL:
(echo foo) >^
> outfile
rem Works as '(echo foo) > outfile' (not '>>'):
(echo foo) >^
 > outfile
rem All types of pre redirection are OK:
> outfile ^
(echo foo)
