@echo off

call purefail.bat
echo purefail [%ERRORLEVEL%]

call fail-exit-errorlevel.bat
echo fail-exit-errorlevel [%ERRORLEVEL%]

call fail-exitb.bat
echo fail-exitb [%ERRORLEVEL%]

call fail-goto.bat
echo fail-goto [%ERRORLEVEL%]

call delfile-exit-errorlevel.bat
echo delfile-exit-errorlevel [%ERRORLEVEL%]

call delfile-exitb.bat
echo delfile-exitb [%ERRORLEVEL%]

call delfile-no-exit.bat
echo delfile-no-exit [%ERRORLEVEL%]
