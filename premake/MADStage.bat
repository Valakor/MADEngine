@echo off

set projectName=%~1
set targetDir=%~2
set solutionDir=%~3
set sourceAssetsDir=%solutionDir%assets
set targetAssetsDir=%targetDir%assets

echo Staging assets for %projectName%...
echo Target Asset Dir=%targetAssetsDir%
echo Source Asset Dir=%sourceAssetsDir%

if NOT exist %targetAssetsDir% (
	echo 	Assets folder not linked, linking now...
	mklink /J %targetAssetsDir% %sourceAssetsDir%
) else (
	echo 	Assets folder already linked.
)
echo Solution staging for %projectName% finished.
