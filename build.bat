call "%VS140COMNTOOLS%VsMSBuildCmd.bat"
msbuild ZeGotoControlCenter.sln /m /p:configuration=Release /p:platform=x64
if %errorlevel% neq 0 goto err
msbuild ZeGotoControlCenter.sln /m /p:configuration=Release /p:platform=x86
if %errorlevel% neq 0 goto err
"C:\Program Files (x86)\Inno Setup 5\Compil32.exe" /cc ZeGotoControlCenter_Setup.iss
if %errorlevel% neq 0 goto err
goto end

:err
pause

:end