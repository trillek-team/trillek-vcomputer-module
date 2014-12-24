# PowerShell script to install all dependencies
#
# Use choco & nuget to grab glew and glfw

new-item lib -type directory
new-item lib\x86 -type directory
new-item lib\x86\Debug -type directory
new-item lib\include -type directory
new-item lib\include\glm -type directory
new-item build -type director

pushd

Start-FileDownload 'https://googletest.googlecode.com/files/gtest-1.7.0.zip'
7z -y x gtest-1.7.0.zip

$env:GTEST_ROOT = $PWD.Path + "\gtest-1.7.0\"

popd
