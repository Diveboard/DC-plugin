set HOME=%HOMEPATH%
cd %TMP%
rd /S /Q diveboard-plugin
"C:\Program Files (x86)\Git\bin\git.exe" clone --depth 1 -- git@diveboard.plan.io:diveboard-db.git "diveboard-plugin"

cd diveboard-plugin
"C:\Program Files (x86)\Git\bin\git.exe" update --init

call firebreath\prep2010.cmd projects build
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
msbuild libdivecomputer\msvc\libdivecomputer.sln /p:Configuration=Release /t:clean,rebuild
msbuild build\FireBreath.sln /p:Configuration=Release /t:clean,rebuild
"C:\Program Files (x86)\NSIS\makensis.exe" projects\DiveBoard\installer.nsi

explorer build\bin\DiveBoard\Release

cd %HOMEPATH%
pause > nul
exit /b
