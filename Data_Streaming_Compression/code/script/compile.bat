@echo off

set "WORK_DIR=%cd%"
@del /F /Q /s bin\* > nul

@cd %WORK_DIR%\bin
for /f %%f in ('dir /s/b /a-d %WORK_DIR%\src\*') do (
    g++ -I %WORK_DIR%\include\ -I %WORK_DIR%\lib\ --std=c++11 -c %%f
)
g++ -I %WORK_DIR%\include\ -I %WORK_DIR%\lib\ --std=c++11 -c %WORK_DIR%\main.cpp

g++ %WORK_DIR%\bin\*.o -o _exe
exit 0