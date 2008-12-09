!define MUI_PRODUCT "FloboPuyo"
!define MUI_VERSION "1.0.0"

;--------------------------------
;Include Modern UI
!include "MUI.nsh"

# define name of installer
Name "FloboPuyo"
outFile "FloboPuyoSetup.exe"

;Folder-selection page
InstallDir "$PROGRAMFILES\${MUI_PRODUCT}"
;Get installation folder from registry if available
InstallDirRegKey HKCU "Software\${MUI_PRODUCT}" ""


;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Configuration

#  !define MUI_FINISHPAGE
#  !define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\FloboPuyo.lnk"

#  !define MUI_LICENSEPAGE
#  !define MUI_COMPONENTSPAGE
#  !define MUI_DIRECTORYPAGE
#  !define MUI_ABORTWARNING
#  !define MUI_UNCONFIRMPAGE
#  !define MUI_UNINSTALLER

#  !define MUI_STARTMENUPAGE

#!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"
#  !define MUI_STARTMENUPAGE_REGISTRY_KEY \
#    "Software\Microsoft\Windows\CurrentVersion\Uninstall\FloboPuyo"
#  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
#  !define MUI_STARTMENUPAGE_VARIABLE $R9

#  !define MUI_HEADERBITMAP "branding.bmp"

#  ;Language
#  !define MUI_TEXT_FINISH_SHOWREADME "Run ${MUI_PRODUCT}"
#  !insertmacro MUI_LANGUAGE "English"

#  ;License page
  #LicenseData "license.rtf"

# start default section
Section "!FloboPuyo" SecCore
SectionIn RO
    SetOutPath $INSTDIR

    # set the installation directory as the destination for the following actions
    setOutPath $INSTDIR

    file flobopuyo.exe

    ;Store installation folder
    WriteRegStr HKCU "Software\${MUI_PRODUCT}" "" $INSTDIR

    # create the uninstaller
    writeUninstaller "$INSTDIR\uninstall.exe"

    # create a shortcut named "new shortcut" in the start menu programs directory
    # point the new shortcut at the program uninstaller
    createShortCut "$SMPROGRAMS\new shortcut.lnk" "$INSTDIR\uninstall.exe"
sectionEnd

;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "Game files"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "An icon of Frozen Bubble \
    on the desktop"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecQuick} "An icon of FloboPuyo \
    in the Quick Launch bar"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecScript} "This installer scripts \
    (requires NSIS)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

# uninstaller section start
section "uninstall"

    # first, delete the uninstaller
    delete "$INSTDIR\uninstall.exe"

    # second, remove the link from the start menu
    delete "$SMPROGRAMS\new shortcut.lnk"

    delete "$INSTDIR\flobopuyo.exe"

# uninstaller section end
sectionEnd
