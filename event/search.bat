@echo off
if "%1" neq "" goto next
mshta vbscript:createobject("wscript.shell").run("search.bat "+clipboarddata.getData("text"))(window.close)
goto :eof

:next
FINDSTR /s %1 *.*
pause>nul
exit