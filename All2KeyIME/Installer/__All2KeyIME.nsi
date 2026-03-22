; IME Installation Script
; Written by quarternote

!define RELVERSION      "0.0.8"
!define APPNAME         "All2key ${RELVERSION}"

!define DLLNAME "all2keyIME.dll"
!define X64SRC "..\cpp\x64\Debug\${DLLNAME}"
!define X86SRC "..\cpp\Win32\Debug\${DLLNAME}"
!define DATSRC "..\cpp\All2KeyIME\Dictionary\dict.txt"
!define INSTDIR "All2keyIME"

!define NAVILIME_ICON "..\cpp\All2KeyIME\Image\SampleIme.ico"
!define NAVILIME_CTRL_TOOL "..\navilIME_control\navilIME_control\bin\Release\navilIME_control.exe"

;!include "x64.nsh"
;Include Modern UI
!include "MUI2.nsh"
;Include Radio buttons
!include "Sections.nsh"

SetCompressor /SOLID lzma

BrandingText "All2KeyIME Installer"

;--------------------------------
;Configuration

  ;General
  Name "${APPNAME}"
  OutFile "All2KeyIME-${RELVERSION}.exe"
  !define MUI_ICON "${NAVILIME_ICON}"
  !define MUI_UNICON "${NAVILIME_ICON}"

;--------------------------------
;Pages

  !insertmacro MUI_DEFAULT MUI_WELCOMEFINISHPAGE_BITMAP "setup.bmp"

  !insertmacro MUI_PAGE_WELCOME 
  !insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH 


  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "All2Key Core" SecBody
  SetOutPath "$PROGRAMFILES64\${INSTDIR}"
  SetOverwrite try
  File "${X64SRC}"
  File "${DATSRC}"
  
  File "${NAVILIME_CTRL_TOOL}"
  File "${NAVILIME_ICON}"
  
  # Start Menu
  CreateDirectory "$SMPROGRAMS\${INSTDIR}"
  CreateShortCut "$SMPROGRAMS\${INSTDIR}\Setting.lnk" "$PROGRAMFILES64\${INSTDIR}\navilIME_control.exe" "" "$PROGRAMFILES64\${INSTDIR}\Navilime.ico" 0

  SetOutPath "$PROGRAMFILES32\${INSTDIR}"
  SetOverwrite try
  File "${X86SRC}"
  File "${DATSRC}"
  
  Exec 'regsvr32 "$PROGRAMFILES64\${INSTDIR}\${DLLNAME}"'
  Exec 'regsvr32 "$PROGRAMFILES32\${INSTDIR}\${DLLNAME}"'
SectionEnd










Section -AdditionalIcons
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
;  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\${PRODUCT_NAME}.exe"
;  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
;  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
;  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\${PRODUCT_NAME}.exe"
;  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
;  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name)는(은) 완전히 제거되었습니다."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "$(^Name)을(를) 제거하시겠습니까?" IDYES +2
  Abort
FunctionEnd



Section Uninstall

  
  ;ExecWait 'msiexec.exe /i "C:\Program Files (x86)\quarternote\SMS\mariadb-10.10.1-winx64.msi" REMOVE=ALL'

  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\${PRODUCT_NAME}.exe"
  

  Delete "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\Website.lnk"
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
  Delete "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk"
  
    Delete "$INSTDIR\uninst.exe"
  
  RMDir /r "$INSTDIR\data"
  RMDir /r "$INSTDIR\upload"
  RMDir "$SMPROGRAMS\${PRODUCT_NAME}"
  RMDir "$INSTDIR"

  Exec 'regsvr32 -u "$PROGRAMFILES64\${INSTDIR}\${DLLNAME}"'
  Exec 'regsvr32 -u "$PROGRAMFILES32\${INSTDIR}\${DLLNAME}"'

;  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
;  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
