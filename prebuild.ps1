# PowerShell script to install all dependencies

new-item lib -type directory
new-item lib\x86 -type directory
new-item lib\x86\Debug -type directory
new-item lib\include -type directory
new-item build -type directory

# Get and build GTest
Start-FileDownload 'http://googletest.googlecode.com/files/gtest-1.7.0.zip'
7z -y x gtest-1.7.0.zip
pushd
Set-Location gtest-1.7.0
cmake -G "Visual Studio 12" .
msbuild gtest.sln /verbosity:minimal /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
popd
move-item -path gtest-1.7.0\include\* -destination lib/include
[Environment]::SetEnvironmentVariable("GTEST_ROOT", "C:\projects\trillek-vcomputer-module\gtest-1.7.0", "Machine")
