!include "MUI2.nsh"
!include "Library.nsh"


#--------------------------------
# Global definition
#--------------------------------

Name "DiveBoard"
Caption "DiveBoard browser plugin"
OutFile "..\..\build\bin\DiveBoard\Debug\DiveBoard-debug.exe"


InstallDir "$COMMONFILES\DiveBoard"
InstallDirRegKey HKLM "Software\DiveBoard" ""

XPStyle on

RequestExecutionLevel admin

!define DLL_LibDiveComputer '"..\..\libdivecomputer\Debug\libdivecomputer.dll"'
!define DLL_DiveBoardPlugin '"..\..\build\bin\DiveBoard\Debug\npDiveBoard.dll"'



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

LangString DESC_Driver1 ${LANG_ENGLISH} "Description of section 1."
LangString DESC_Driver2 ${LANG_ENGLISH} "Description of section 2."
LangString DESC_Driver3 ${LANG_ENGLISH} "Description of section 3."



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

Section

SetShellVarContext all
CreateDirectory "$INSTDIR"
!insertmacro InstallLib DLL       NOTSHARED NOREBOOT_NOTPROTECTED ${DLL_LibDiveComputer} $INSTDIR\libdivecomputer.dll $INSTDIR
!insertmacro InstallLib REGDLL    NOTSHARED NOREBOOT_NOTPROTECTED ${DLL_DiveBoardPlugin} $INSTDIR\npDiveBoard.dll $INSTDIR
WriteUninstaller $INSTDIR\uninstall.exe
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibTest" "DisplayName" "TestLibrary"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibTest" "DisplayVersion" "1.0"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibTest" "UninstallString" "$INSTDIR\uninstall.exe"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibTest" "InstallLocation" "$INSTDIR"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibTest" "NoModify" "1"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibTest" "NoRepair" "1"

SectionEnd


#---  Low level drivers ---

Section "FTDI" Driver1
        SetOutPath $TEMP\DB_FTDI
	File /r "..\..\drivers\ftdi_win"
	ExecWait 'rundll32 syssetup,SetupInfObjectInstallAction DefaultInstall 128 "$TEMP\DB_FTDI\ftdi_win\ftdibus.inf"' $0
	# MessageBox MB_OK "FTDI Driver installed. Installer returned $0.\n $TEMP" IDOK 0

	Delete "$TEMP\DB_FTDI"
SectionEnd

Section "SiliconLabs CP210x" Driver2
        SetOutPath $TEMP\DB_SILABS
	File /r "..\..\drivers\Silabs_windows"
	ExecWait '"$TEMP\DB_SILABS\Silabs_windows\CP210x_VCP_Win2K.exe"' $0

	# MessageBox MB_OK "SiliconLabs CP210x Driver installed. Installer returned $0.\n $TEMP" IDOK 0

	Delete "$TEMP\DB_SILABS"
SectionEnd

Section "Prolific" Driver3
        SetOutPath $TEMP\DB_PROLIFIC
	File /r "..\..\drivers\prolific_win"
	ExecWait '"$TEMP\DB_PROLIFIC\prolific_win\PL2303_Prolific_DriverInstaller_v130.exe"' $0

	# MessageBox MB_OK "Prolific Driver installed. Installer returned $0 \n $TEMP" IDOK 0

	Delete "$TEMP\DB_PROLIFIC"
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


DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\LibTest" 
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
