# TODO set GTEST_ROOT to were is located gtest
cmake -G "Visual Studio 12" -DCMAKE_BUILD_TYPE:String=Debug -DBUILD_STATIC_VCOMPUTER=True -DBUILD_TOOLS_VCOMPUTER=False .

msbuild VCOMPUTER.sln /p:Configuration=Debug /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"

