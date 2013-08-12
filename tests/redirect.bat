@echo off
echo 1 > r1
> f1 :ech 2
echo 3 >r3 2>r4
echo 15 ^>f1
echo 15 ^>^>f1

rem FAIL (Prints 'foo > outfile'):
echo foo ^
> outfile

rem OK (any spaces before '>' are OK):
echo foo ^
 > outfile

rem OK
echo foo >^
outfile

rem Works as 'echo foo > " outfile"':
echo foo >^
 outfile

rem FAIL ("bad syntax"). 2 or more spaces before 'outfile' not OK:
echo foo >^
  outfile

>outfile echo 33
echo 44 >outfile 55
echo==========>outfile ====66====77