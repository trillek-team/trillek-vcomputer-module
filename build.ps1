cmake -version

cmake -G "Visual Studio 12" -DUSE_STATIC_GLEW=True .

msbuild VCOMPUTER.sln /p:Configuration=Debug /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"


