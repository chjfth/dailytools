@echo off
if ""=="%*" exit /b 0

set param=%*
set paramEscd=%param:"=""%
echo "%paramEscd%"