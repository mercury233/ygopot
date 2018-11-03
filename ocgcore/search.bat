@echo off
if "%1" neq "" goto next
mshta vbscript:createobject("wscript.shell").run("search.bat "+clipboarddata.getData("text"))(window.close)
goto :eof

:next
echo 1234>aaa.txt
FINDSTR /s %1 *.*>>aaa.txt
aaa.txt
exit