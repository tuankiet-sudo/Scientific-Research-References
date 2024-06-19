@echo off

set "WORK_DIR=%cd%"
set "PLOT_CONFIG=%WORK_DIR%\config\plot.json"
set "ALGO_CONFIG=%WORK_DIR%\config\algo.json"

@REM Parsing json config
for /f "delims=" %%i in ('python %WORK_DIR%\python\parse_algo.py %ALGO_CONFIG%') do set TEMP=%%i
%WORK_DIR%\bin\_exe.exe %WORK_DIR%\%TEMP%
del /F /Q %WORK_DIR%\%TEMP% > nul

@REM Plotting all results
python "%WORK_DIR%\python\plot.py" %PLOT_CONFIG%
exit 0