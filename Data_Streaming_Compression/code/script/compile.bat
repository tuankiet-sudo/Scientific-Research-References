@echo off

set "WORK_DIR=%cd%"
@del /F /Q /s bin\* > nul

@mkdir %WORK_DIR%\bin\obj
@cd %WORK_DIR%\bin\obj
for /f %%f in ('dir /s/b /a-d %WORK_DIR%\src\*') do (
    g++ -I %WORK_DIR%\include\ -I %WORK_DIR%\lib\ --std=c++11 -c %%f
)

@cd %WORK_DIR%\bin
g++ -I %WORK_DIR%\include\ -I %WORK_DIR%\lib\ --std=c++11 -c %WORK_DIR%\compress.cpp
g++ -I %WORK_DIR%\include\ -I %WORK_DIR%\lib\ --std=c++11 -c %WORK_DIR%\decompress.cpp

g++ %WORK_DIR%\bin\obj\*.o %WORK_DIR%\bin\compress.o -o compress
g++ %WORK_DIR%\bin\obj\*.o %WORK_DIR%\bin\decompress.o -o decompress
exit 0