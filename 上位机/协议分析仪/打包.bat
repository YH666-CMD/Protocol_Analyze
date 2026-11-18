@echo off
cd /d "%~dp0"
py -3 -m pip install pyserial pyinstaller
py -3 -m PyInstaller --noconfirm --clean --windowed --onefile --name 协议分析仪 --hidden-import serial.tools.list_ports main.py
echo.
echo 生成文件：dist\协议分析仪.exe
echo 把 exe 放在本目录旁边运行时，会到上两级的 工具\加密工具\密钥.txt 找密钥。
echo 也可以把 密钥.txt 复制到 exe 旁边。
pause
