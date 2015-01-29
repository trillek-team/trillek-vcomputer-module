$env:GTEST_ROOT = $PWD.Path + "\gtest-1.7.0\"
cmake -G "Visual Studio 12" -DCMAKE_BUILD_TYPE=Debug -DBUILD_TOOLS_VCOMPUTER=False .

msbuild VCOMPUTER.sln /p:Configuration=Debug /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"

cd bin
cd Debug
unit_test.exe

