!include "MUI.nsh"

  ;Name and file
  Name "Minidjvu 0.8"
  OutFile "minidjvu-0.8.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\minidjvu"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\minidjvu" ""


;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "Russian"

;--------------------------------
;Installer Sections

Section "Install"

  SetOutPath "$INSTDIR"
  
  Delete "$INSTDIR\bash.dll"
  Delete "$INSTDIR\bash.exe"
  Delete "$INSTDIR\djvm.exe"
  Delete "$INSTDIR\jpeg_lic.html"
  Delete "$INSTDIR\jpeg62.dll"
  Delete "$INSTDIR\libtiff3.dll"
  Delete "$INSTDIR\rm.exe"
  Delete "$INSTDIR\tiff_lic.html"
  Delete "$INSTDIR\tiffinfo.exe"
  Delete "$INSTDIR\tiffsplit.exe"
  Delete "$INSTDIR\tifftodjvu.hta"
  Delete "$INSTDIR\tifftodjvu_help.html"
  Delete "$INSTDIR\windjview.exe"
  Delete "$INSTDIR\zlib1.dll"
  Delete "$INSTDIR\README.html"
  Delete "$SMPROGRAMS\minidjvu\README.lnk"
  Delete "$INSTDIR\minidjvu.nsi"
  
  File "COPYING"
  File "News"
  File "bin\minidjvu.exe"
  File "tifftodjvu.hta"
  CreateDirectory $INSTDIR\doc
  File "/oname=doc\tifftodjvu_help.html" "doc\tifftodjvu_help.html"

  CreateDirectory $SMPROGRAMS\minidjvu
  CreateShortcut  $SMPROGRAMS\minidjvu\TIFF-to-DjVu.lnk $INSTDIR\tifftodjvu.hta
  CreateShortcut  $SMPROGRAMS\minidjvu\Uninstall.lnk $INSTDIR\Uninstall.exe

  ;Store installation folder
  WriteRegStr HKCU "Software\minidjvu" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\COPYING"
  Delete "$INSTDIR\minidjvu.exe"
  Delete "$INSTDIR\tifftodjvu.hta"
  Delete "$INSTDIR\doc\tifftodjvu_help.html"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR\doc"
  RMDir "$INSTDIR"

  Delete "$SMPROGRAMS\minidjvu\TIFF-to-DjVu.lnk"
  Delete "$SMPROGRAMS\minidjvu\Uninstall.lnk"
  RMDir  "$SMPROGRAMS\minidjvu"

  DeleteRegKey /ifempty HKCU "Software\minidjvu"
SectionEnd
