@echo off
REM Tips from https://www.robvanderwoude.com/clevertricks.php
REM Exitcode=0 means yes.
:IsRunAsAdmin
whoami /groups | find "12288" >nul 2>nul
