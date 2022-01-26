@echo off

REM Thanks to: https://stackoverflow.com/a/5841587/151453

if not "%~1" == "" (
	REM Note: String content should be placed into a bat-var and you 
	REM pass in varname as parameter. So the string can contain space-chars.
	call :strlen %~1 %~2
	exit /b 0
)

setlocal
REM *** Some tests, to check the functionality ***
REM *** An emptyStr has the length 0
set "emptyString="
call :strlen result emptyString
echo %result%

REM *** This string has the length 14
set "myString=abcdef!%%^^()^!"
call :strlen result myString
echo %result%

REM *** This string has the maximum length of 8191
setlocal EnableDelayedExpansion
set "long=."
FOR /L %%n in (1 1 13) DO set "long=!long:~-4000!!long:~-4000!"
(set^ longString=!long!!long:~-191!)

call :strlen result longString
echo %result%

goto :eof

REM ********* function *****************************
:strlen <resultVar> <stringVar>
(   
    setlocal EnableDelayedExpansion
    (set^ tmp=!%~2!)
    if defined tmp (
        set "len=1"
        for %%P in (4096 2048 1024 512 256 128 64 32 16 8 4 2 1) do (
            if "!tmp:~%%P,1!" NEQ "" ( 
                set /a "len+=%%P"
                set "tmp=!tmp:~%%P!"
            )
        )
    ) ELSE (
        set len=0
    )
)
( 
    endlocal
    set "%~1=%len%"
    exit /b
)