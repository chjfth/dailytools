@echo off
REM This is a function. It packs(escapes) %* to become a SINGLE parameter,
REM and this SINGLE parameter can be passed to inner bat as single paramenter.
REM What it does is very simple, just replace single double-quote(DQ) to double DQ.
REM Varname DQescape_NoTBS_Output contains the output value.
REM When the caller expands it, just write %DQescape_NoTBS_Output%, do NOT 
REM re-wrap double-quotes.
REM
REM Caveat: Any parameter to pack MUST NOT have trailing backslash, as _NoTBS
REM in this bat filename implies. Otherwise, it will not work correctly.
REM
REM When inner bat receive it as, for example, %1 parameter, the inner guy
REM should unescape it, i.e. replacing double DQ back to a single DQ.
REM
REM Useful historical info:
REM https://docs.microsoft.com/en-us/archive/blogs/twistylittlepassagesallalike/everyone-quotes-command-line-arguments-the-wrong-way

:DQescape_NoTBS

set DQescape_NoTBS_Output=

setlocal

if ""=="%~1" exit /b 0

set param=%*
set paramEscd=%param:"=""%
set ret="%paramEscd%"

endlocal & (
    set DQescape_NoTBS_Output=%ret%
)
