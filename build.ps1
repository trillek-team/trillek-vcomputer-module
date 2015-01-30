cmake

cmake -G "Visual Studio 12"  -DBUILD_TOOLS_VCOMPUTER=False .

msbuild VCOMPUTER.sln /p:Configuration=Debug /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"

cd bin
cd Debug
unit_test

