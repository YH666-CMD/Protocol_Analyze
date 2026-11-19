@echo off
cd /d "%~dp0"
py -3 -m pip install pyserial pyinstaller
py -3 pack.py
if errorlevel 1 pause
pause
