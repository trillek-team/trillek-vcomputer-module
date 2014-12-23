# PowerShell script to install all dependencies
#
# Use choco & nuget to grab glew and glfw
choco install NuGet.CommandLine

new-item lib -type directory
new-item lib\x86 -type directory
new-item lib\x86\Debug -type directory
new-item lib\include -type directory
new-item lib\include\glm -type directory
new-item build -type director

pushd
# glew
# C:\glew.1.9.0.1\build\native
#nuget install glew 
#Copy-Item -path c:\glew.1.9.0.1\include\* -destination lib/include

#nuget install GLFW
#nuget install glm
#nuget install openal.redist
nuget install gtest
$env:GTEST_ROOT = "C:\gtest.1.7.0.1\"

popd
