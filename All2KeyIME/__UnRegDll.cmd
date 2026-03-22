
set IME_CLSID={01234567-89AB-CDEF-0123-456789ABCDEF}


regsvr32 /u /s "c:\\Users\\devscott\\Desktop\\All2KeyIME\\x64\\debug\\All2keyIME.dll"

taskkill /f /im ctfmon.exe
taskkill /f /im explorer.exe

del  "c:\Users\devscott\Desktop\All2KeyIME\\x64\debug\All2KeyIME.dll"

start ctfmon.exe
start explorer.exe

pause

