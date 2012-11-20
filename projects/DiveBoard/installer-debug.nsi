!include "MUI2.nsh"
!include "Library.nsh"


#--------------------------------
# Global definition
#--------------------------------

Name "DiveBoard"
Caption "DiveBoard browser plugin"
OutFile "..\..\build\bin\DiveBoard\Debug\DiveBoard-debug.exe"
Icon "logo.ico"
UninstallIcon "logo.ico"


InstallDir "$COMMONFILES\DiveBoard"
InstallDirRegKey HKLM "Software\DiveBoard" ""

XPStyle on

RequestExecutionLevel admin

!define DLL_LibDiveComputer '"..\..\libdivecomputer\msvc\Debug\libdivecomputer.dll"'
!define DLL_DiveBoardPlugin '"..\..\build\bin\DiveBoard\Debug\npDiveBoard.dll"'
!define DLL_MSVCR '"Win\msvcr100.dll"'



#--------------------------------
# Pages
#--------------------------------


!define MUI_ABORTWARNING

!define MUI_PAGE_CUSTOMFUNCTION_PRE checkRights

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

LangString DESC_Plugin  ${LANG_ENGLISH} "Web browser Plugin to upload your dives directly from your computer to Diveboard.com."
LangString DESC_Libdive ${LANG_ENGLISH} "Libdivecomputer is a cross-platform and open source (LGPL) library for communication with dive computers from various manufacturers. divesoftware.org/libdc "
LangString DESC_Driver1 ${LANG_ENGLISH} "FTDI Driver for Suunto and Oceanic computers. Windows usually auto-detects these devices, so it should not be necessary to install this driver."
LangString DESC_Driver2 ${LANG_ENGLISH} "Silicon driver for Mares IR device. Windows usually auto-detects these devices, so it should not be necessary to install this driver."
LangString DESC_Driver3 ${LANG_ENGLISH} "Prolific driver for Reefnet, Cressi, Zeagle. Windows usually auto-detects these devices, so it should not be necessary to install this driver."



#--------------------------------
# Admin rights check
#--------------------------------

Function checkRights

	ClearErrors
	UserInfo::GetName
	IfErrors Win9x
	Pop $0
	UserInfo::GetAccountType
	Pop $1
	UserInfo::GetOriginalAccountType
	Pop $2
	# GetOriginalAccountType will check the tokens of the original user of the
	# current thread/process. If the user tokens were elevated or limited for
	# this process, GetOriginalAccountType will return the non-restricted
	# account type.
	# On Vista with UAC, for example, this is not the same value when running
	# with `RequestExecutionLevel user`. GetOriginalAccountType will return
	# "admin" while GetAccountType will return "user".

	# StrCmp $2 "Admin" 0 +3
	StrCmp $2 "Power" 0 +3
		MessageBox MB_OK 'You need to have administration rights to install this software.'
		Quit
	StrCmp $2 "User" 0 +3
		MessageBox MB_OK 'You need to have administration rights to install this software.'
		Quit
	StrCmp $2 "Guest" 0 +3
		MessageBox MB_OK 'You need to have administration rights to install this software.'
		Quit

	Win9x:

	done:
FunctionEnd


#--------------------------------
# Main Installation commands
#--------------------------------

#---  Main libraries ---

Section "!Diveboard plugin" "plugin"
SectionIn 1 RO

SetShellVarContext all


Var /GLOBAL PREINSTALL

ClearErrors
ReadRegStr $PREINSTALL HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DiveBoard" "InstallLocation"
IfErrors FoundInstalled 0

RMDll1:
ClearErrors
Delete $PREINSTALL\npDiveBoard.dll
IfErrors 0 NoError1
  MessageBox MB_OK "The library is locked. Please close all browsers and retry"
  Goto RMDll1
NoError1:

RMDll2:
ClearErrors
Delete $PREINSTALL\libdivecomputer.dll
IfErrors 0 NoError2
  MessageBox MB_OK "The library is locked. Please close all browsers and retry"
  Goto RMDll2
NoError2:

RMDll3:
ClearErrors
Delete $PREINSTALL\msvcr100.dll
IfErrors 0 NoError3
  MessageBox MB_OK "The library is locked. Please close all browsers and retry"
  Goto RMDll3
NoError3:

FoundInstalled:

RMDll4:
ClearErrors
Delete $INSTDIR\npDiveBoard.dll
IfErrors 0 NoError4
  MessageBox MB_OK "The library is locked. Please close all browsers and retry"
  Goto RMDll4
NoError4:

RMDll5:
ClearErrors
Delete $INSTDIR\libdivecomputer.dll
IfErrors 0 NoError5
  MessageBox MB_OK "The library is locked. Please close all browsers and retry"
  Goto RMDll5
NoError5:

RMDll6:
ClearErrors
Delete $INSTDIR\msvcr100.dll
IfErrors 0 NoError6
  MessageBox MB_OK "The library is locked. Please close all browsers and retry"
  Goto RMDll6
NoError6:

CreateDirectory "$INSTDIR"
!insertmacro InstallLib DLL       NOTSHARED NOREBOOT_NOTPROTECTED ${DLL_LibDiveComputer} $INSTDIR\libdivecomputer.dll $INSTDIR
!insertmacro InstallLib REGDLL    NOTSHARED NOREBOOT_NOTPROTECTED ${DLL_DiveBoardPlugin} $INSTDIR\npDiveBoard.dll $INSTDIR
!insertmacro InstallLib DLL       NOTSHARED NOREBOOT_NOTPROTECTED ${DLL_msvcr} $INSTDIR\msvcr100.dll $INSTDIR
WriteUninstaller $INSTDIR\uninstall.exe
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DiveBoard" "DisplayName" "DiveBoard"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DiveBoard" "DisplayVersion" "1.0"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DiveBoard" "UninstallString" "$INSTDIR\uninstall.exe"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DiveBoard" "InstallLocation" "$INSTDIR"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DiveBoard" "NoModify" "1"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DiveBoard" "NoRepair" "1"

SectionEnd


Section "Libdivecomputer" "libdivecomputer"
SectionIn 1 RO
SectionEnd


#---  Low level drivers ---

Section /o "FTDI" Driver1
        SetOutPath $TEMP\DB_FTDI
	File /r "..\..\drivers\ftdi_win"
	ExecWait '"$TEMP\DB_FTDI\ftdi_win\CDM20824_Setup.exe"'
	# MessageBox MB_OK "FTDI Driver installed. Installer returned $0.\n $TEMP" IDOK 0

	Delete "$TEMP\DB_FTDI"
SectionEnd

Section /o "SiliconLabs CP210x" Driver2
        SetOutPath $TEMP\DB_SILABS
	File /r "..\..\drivers\Silabs_windows"
	${If} ${RunningX64}
	   ExecWait '"$TEMP\DB_SILABS\Silabs_windows\CP210xVCPInstaller_x64.exe"' $0
	${Else}
	   ExecWait '"$TEMP\DB_SILABS\Silabs_windows\CP210xVCPInstaller_x86.exe"' $0
	${EndIf}

	# MessageBox MB_OK "SiliconLabs CP210x Driver installed. Installer returned $0.\n $TEMP" IDOK 0

	Delete "$TEMP\DB_SILABS"
SectionEnd

Section /o "Prolific" Driver3
        SetOutPath $TEMP\DB_PROLIFIC
	File /r "..\..\drivers\prolific_win"
	ExecWait '"$TEMP\DB_PROLIFIC\prolific_win\PL2303_Prolific_DriverInstaller_v130.exe"' $0

	# MessageBox MB_OK "Prolific Driver installed. Installer returned $0 \n $TEMP" IDOK 0

	Delete "$TEMP\DB_PROLIFIC"
SectionEnd


!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${plugin} $(DESC_Plugin)
  !insertmacro MUI_DESCRIPTION_TEXT ${libdivecomputer} $(DESC_libdive)
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

Udll3:
!insertmacro UninstallLib DLL       NOTSHARED NOREBOOT_NOTPROTECTED  $INSTDIR\msvcr100.dll
IfFileExists "$INSTDIR\msvcr100.dll" 0 NoErrorMsgD3
  MessageBox MB_RETRYCANCEL "The library could not be removed! Please close all browsers and retry" IDRETRY  0  IDCANCEL cancelUninst   ; skipped if file doesn't exist
  goto Udll3
NoErrorMsgD3:


DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DiveBoard" 
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
