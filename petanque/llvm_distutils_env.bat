call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
SET MSSdk=1
SET DISTUTILS_USE_SDK=1
FOR /F "skip=2 tokens=3,*" %%A IN ('reg query "HKLM\Software\LLVM\LLVM" /reg:32 /ve') DO set "LLVM_PATH=%%B"
echo LLVM found in %LLVM_PATH%
SET PATH=%LLVM_PATH%\msbuild-bin;%PATH%
cmd.exe /k