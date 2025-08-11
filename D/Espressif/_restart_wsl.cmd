@echo off

cls

if %VIEW_STATE_ONLY%. NEQ . goto VIEW_STATE_ONLY

echo Port: %COM_PORT%
echo Choose: [92m1 - View state Only
echo         [91m0 - Shutdown + Restart
echo         [93mEnter - Skip + Exit[0m
set /p RESPONSE= 
rem -- Pressed <ENTER>
if %RESPONSE%. == . goto FINISHED
rem -- Go to :VIEW_STATE_ONLY
if %RESPONSE% == 1 goto VIEW_STATE_ONLY

wsl.exe --shutdown
wsl.exe
exit

:VIEW_STATE_ONLY

echo [92m
echo *********************
echo *                   *
echo *   VIEW STATE...   *
echo *                   *
echo *********************
echo [0m
wsl.exe -l -v
pause


:FINISHED
echo [93m
echo ***************************
echo *                         *
echo *   BYE, SEE YOU LATER!   *
echo *                         *
echo ***************************
echo [0m
