@echo off

@REM set "WORK_DIR=%cd%"
@REM set "PLOT_CONFIG=%WORK_DIR%\config\plot.json"
@REM set "ALGO_CONFIG=%WORK_DIR%\config\algo.json"

@REM @REM Parsing json config
@REM for /f "delims=" %%i in ('python %WORK_DIR%\python\parse_algo.py %ALGO_CONFIG%') do set TEMP=%%i
@REM %WORK_DIR%\bin\_exe.exe %WORK_DIR%\%TEMP%
@REM del /F /Q %WORK_DIR%\%TEMP% > nul

@REM @REM Plotting all results
@REM python "%WORK_DIR%\python\plot.py" %PLOT_CONFIG%
@REM exit 0

if %1=="compile" (
    echo "compile"
)
else if %1=="compress" (
    echo "compress"
)
