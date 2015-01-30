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

Start-FileDownload 'http://cpu.zardoz.es/trillek-win32-lib.zip'
7z -y x trillek-win32-lib.zip

popd
