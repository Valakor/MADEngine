echo off

pushd .\premake
premake5.exe vs2015
popd

if %ERRORLEVEL% NEQ 0 (
	echo Build file generation failed
	pause
	exit /b 1
)
