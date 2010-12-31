!include "MUI2.nsh"
!include "Library.nsh"


#--------------------------------
# Global definition
#--------------------------------

Name "DiveBoard"
Caption "DiveBoard browser plugin"
OutFile "..\..\build\bin\DiveBoard\Debug\DiveBoard-debug.exe"


InstallDir "$APPDATA\DiveBoard"
InstallDirRegKey HKCU "Software\DiveBoard" ""

XPStyle on

RequestExecutionLevel user

!define DLL_LibDiveComputer '"..\..\build\bin\DiveBoard\Debug\libdivecomputer.dll"'
!define DLL_DiveBoardPlugin '"..\..\build\bin\DiveBoard\Debug\npDiveBoard.dll"'


#--------------------------------
# Pages
#--------------------------------


!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
#  !insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Docs\Modern UI\License.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"


#--------------------------------
# Languages
#--------------------------------

LangString DESC_Driver1 ${LANG_ENGLISH} "Description of section 1."
LangString DESC_Driver2 ${LANG_ENGLISH} "Description of section 2."
LangString DESC_Driver3 ${LANG_ENGLISH} "Description of section 3."



#--------------------------------
# Main Installation commands
#--------------------------------

#---  Main libraries ---

Section

CreateDirectory "$INSTDIR"
!insertmacro InstallLib DLL       NOTSHARED NOREBOOT_NOTPROTECTED ${DLL_LibDiveComputer} $INSTDIR\libdivecomputer.dll $INSTDIR
!insertmacro InstallLib REGDLL    NOTSHARED NOREBOOT_NOTPROTECTED ${DLL_DiveBoardPlugin} $INSTDIR\npDiveBoard.dll $INSTDIR
WriteUninstaller $INSTDIR\uninstall.exe
WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibTest" "DisplayName" "TestLibrary"
WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibTest" "DisplayVersion" "1.0"
WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibTest" "UninstallString" "$INSTDIR\uninstall.exe"
WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibTest" "InstallLocation" "$INSTDIR"
WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibTest" "NoModify" "1"
WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibTest" "NoRepair" "1"

SectionEnd


#---  Low level drivers ---

Section "FTDI" Driver1
	MessageBox MB_OK "on track" IDOK 0
SectionEnd

Section "SiliconLabs CP210x" Driver2
	MessageBox MB_OK "on track" IDOK 0
SectionEnd

Section "Prolific" Driver3
	MessageBox MB_OK "on track" IDOK 0
SectionEnd


!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${Driver1} $(DESC_Driver1)
  !insertmacro MUI_DESCRIPTION_TEXT ${Driver2} $(DESC_Driver2)
  !insertmacro MUI_DESCRIPTION_TEXT ${Driver3} $(DESC_Driver3)
!insertmacro MUI_FUNCTION_DESCRIPTION_END



#--------------------------------
# UnInstallation commands
#--------------------------------

Section uninstall

Udll1:
!insertmacro UninstallLib DLL       NOTSHARED NOREBOOT_NOTPROTECTED  $INSTDIR\libdivecomputer.dll
IfFileExists "$INSTDIR\libdivecomputer.dll" 0 NoErrorMsgD1
  MessageBox MB_RETRYCANCEL "The library could not be removed! Please close all browsers and retry" IDRETRY  0  IDCANCEL cancelUninst   ; skipped if file doesn't exist
  goto Udll1
NoErrorMsgD1:

Udll2:
!insertmacro UninstallLib REGDLL    NOTSHARED NOREBOOT_NOTPROTECTED  $INSTDIR\npDiveBoard.dll
IfFileExists "$INSTDIR\npDiveBoard.dll" 0 NoErrorMsgD2
  MessageBox MB_RETRYCANCEL "The library could not be removed! Please close all browsers and retry" IDRETRY 0 IDCANCEL cancelUninst ; skipped if file doesn't exist
  goto Udll2
NoErrorMsgD2:


DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibTest" 
DeleteRegKey HKLM "SOFTWARE\NSISTest\BigNSISTest"
Delete "$INSTDIR\uninstall.exe"
RMDir "$INSTDIR"

IfFileExists "$INSTDIR" 0 NoErrorMsgINST
  MessageBox MB_OK "Note: $INSTDIR could not be removed!" IDOK 0 ; skipped if file doesn't exist
NoErrorMsgINST:

Goto end

cancelUninst:
  Abort



end:
SectionEnd
