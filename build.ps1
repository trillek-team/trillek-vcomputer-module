# TODO set GTEST_ROOT to were is located gtest
cmake -G "Visual Studio 12" -D CMAKE_BUILD_TYPE:String=Release -DBUILD_STATIC_VCOMPUTER=True .
msbuild VCOMPUTER.sln /p:Configuration=Release /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"

