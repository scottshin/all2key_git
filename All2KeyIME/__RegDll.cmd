cd "C:\Users\devscott\Desktop\backup\All2KeyIME"

"c:\Program Files\Microsoft Visual Studio\18\Insiders\MSBuild\Current\Bin\MSBuild.exe" ./All2KeyIME.sln


regsvr32 "c:\\Users\\devscott\\Desktop\\backup\\All2KeyIME\\x64\debug\All2keyIME.dll"

taskkill /f /im ctfmon.exe
ctfmon.exe

pause

