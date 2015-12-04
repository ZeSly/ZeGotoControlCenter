rem copy /Y "%QTDIR%\bin\icudt52.dll" %1
rem copy /Y "%QTDIR%\bin\icuin52.dll" %1
rem copy /Y "%QTDIR%\bin\icuuc52.dll" %1
copy /Y "%QTDIR%\bin\Qt5Core.dll" %1
copy /Y "%QTDIR%\bin\Qt5Gui.dll" %1
copy /Y "%QTDIR%\bin\Qt5Network.dll" %1
copy /Y "%QTDIR%\bin\Qt5Widgets.dll" %1
copy /Y "%QTDIR%\bin\Qt5SerialPort.dll" %1
rem copy /Y "%QTDIR%\bin\libEGL.dll" %1
copy /Y "%QTDIR%\bin\libGLESv2.dll" %1
if not exist %1\platforms mkdir %1\platforms
copy /Y "%QTDIR%\plugins\platforms\qwindows.dll" %1\platforms
