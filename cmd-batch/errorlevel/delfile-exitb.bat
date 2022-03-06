@echo off
echo abc > normal.txt

REM This will delete normal.txt and have ERRORLEVEL feedback.
REM Thanks to: https://stackoverflow.com/a/33403497/151453
> nul ver & for /F "tokens=*" %%# in ('del /Q "normal.txt" 2^>^&1 1^> nul') do (2> nul set =)

exit /b
