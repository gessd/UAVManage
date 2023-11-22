# ��װ������ѹ
!packhdr temp.dat "cmd /c Copy /B temp.dat /B +Nanlon.7z temp.dat"
#=================================================================

# ͷ�ļ�Ŀ¼
!addincludedir ".\Include"

!include "MUI2.nsh"
!include "x64.nsh"
!include "WordFunc.nsh"

; �ýű�ʹ�� HM VNISEdit �ű��༭���򵼲���
/*** ��װ���� ***/
# �����ļ����Ǳ��
SetOverwrite try
# ����ѹ��ѡ��
SetCompress auto
# ѡ��ѹ����ʽ
SetCompressor /SOLID lzma
SetCompressorDictSize 32
# �������ݿ��Ż�
SetDatablockOptimize on
# ������������д���ļ�ʱ��
SetDateSave on
# ����Ӧ�ó������ԱȨ��
RequestExecutionLevel admin
# �Ƿ�����װ�ڸ�Ŀ¼��
AllowRootDirInstall false
# �����Ƿ���ʾ��װ��ϸ��Ϣ
ShowInstDetails hide 
# �����Ƿ���ʾж����ϸ��Ϣ
ShowUnInstDetails hide 

; ��װ�����ʼ���峣��
!define PRODUCT_NAME "���˻�������"											
!define PRODUCT_VERSION "2.2.0"
!define PRODUCT_PUBLISHER "������Ԫ"
!define PRODUCT_WEB_SITE ""
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\UAVManage.exe"
;ѹ���㷨
SetCompressor lzma
                                   
; ------ MUI �ִ����涨�� (1.67 �汾���ϼ���) ------
!include "MUI.nsh"

; MUI Ԥ���峣��
!define MUI_ABORTWARNING
!define MUI_ICON "..\UAVManage\res\logo\qz_logo.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP ".\Resource\nsis3-grey.bmp"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; ��ӭҳ��
!insertmacro MUI_PAGE_WELCOME
; ���Э��ҳ��!insertmacro MUI_PAGE_LICENSE "c:\path\to\licence\YourSoftwareLicence.txt"
; ���ѡ��ҳ��!insertmacro MUI_PAGE_COMPONENTS
; ��װĿ¼ѡ��ҳ��
!define MUI_PAGE_CUSTOMFUNCTION_SHOW mulu
!insertmacro MUI_PAGE_DIRECTORY
; ��װ����ҳ��
!insertmacro MUI_PAGE_INSTFILES
; ��װ���ҳ��
;!define MUI_FINISHPAGE_SHOWREADME
;!define MUI_FINISHPAGE_SHOWREADME_Function installDrivers
;!define MUI_FINISHPAGE_SHOWREADME_TEXT "�����Զ�����"
!define MUI_FINISHPAGE_RUN "$INSTDIR\UAVManage.exe"
!define MUI_FINISHPAGE_RUN_NOTCHECKED
!insertmacro MUI_PAGE_FINISH

; ��װж�ع���ҳ��
!insertmacro MUI_UNPAGE_INSTFILES

; ��װ�����������������
!insertmacro MUI_LANGUAGE "SimpChinese"



; ��װԤ�ͷ��ļ�
;!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
; ------ MUI �ִ����涨����� ------

Name "${PRODUCT_NAME}"
OutFile "${PRODUCT_NAME}_v${PRODUCT_VERSION}.exe"
InstallDir "D:\UAVManage"
InstallDirRegKey HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
ShowInstDetails show
ShowUnInstDetails show

!define appDir "..\x64\Release"

Var File3DCount
Var CP2102
Var CH340
Section "MainSection" SEC01
	;��ȡ·������ļ�������
    ;StrCpy $0 "$INSTDIR"
    ;StrCpy $1 0
    ;loop:
    ;    IntOp $1 $1 - 1 ; Character offset, from end of string
    ;    StrCpy $2 $0 1 $1 ; Read 1 character into $2, -$1 offset from end
    ;    StrCmp $2 '\' found
    ;    StrCmp $2 '' stop loop ; No more characters or try again
    ;found:
    ;    IntOp $1 $1 + 1 ; Don't include / in extracted string
    ;stop:
    ;StrCpy $2 $0 "" $1 ; We know the length, extract final string part
    ;MessageBox MB_OK|MB_ICONEXCLAMATION "$2"

	; ��ȫ��֤
	SetOutPath "$INSTDIR"
	SetOverwrite on
	File /r ".\CH340����"
	File /r ".\CP2102����"
	File /r "${appDir}\3D"
	File /r "${appDir}\7z"
	File /r "${appDir}\blockly_dev"
	File /r "${appDir}\imageformats"
	File /r "${appDir}\mediaservice"
	File /r "${appDir}\platforms"
	File /r "..\UAVManage\python38"
	File /r "${appDir}\resources"
	File /r "${appDir}\styles"
	File /r "${appDir}\translations"
	File /r "${appDir}\tools"
	File /r "${appDir}\update"
	File    "${appDir}\libEGL.dll"
	File    "${appDir}\libGLESv2.dll"
	File    "${appDir}\opengl32sw.dll"
	File    "${appDir}\python3.dll"
	File    "${appDir}\python38.dll"
	File    "${appDir}\Qt5Core.dll"
	File    "${appDir}\Qt5Gui.dll"
	File    "${appDir}\Qt5Multimedia.dll"
	File    "${appDir}\Qt5Network.dll"
	File    "${appDir}\Qt5Positioning.dll"
	File    "${appDir}\Qt5PositioningQuick.dll"
	File    "${appDir}\Qt5PrintSupport.dll"
	File    "${appDir}\Qt5Qml.dll"
	File    "${appDir}\Qt5QmlModels.dll"
	File    "${appDir}\Qt5Quick.dll"
	File    "${appDir}\Qt5QuickWidgets.dll"
	File    "${appDir}\Qt5SerialPort.dll"
	File    "${appDir}\Qt5Svg.dll"
	File    "${appDir}\Qt5WebChannel.dll"
	File    "${appDir}\Qt5WebEngine.dll"
	File    "${appDir}\Qt5WebEngineCore.dll"
	File    "${appDir}\Qt5WebEngineWidgets.dll"
	File    "${appDir}\Qt5WebSockets.dll"
	File    "${appDir}\Qt5WebView.dll"
	File    "${appDir}\Qt5Widgets.dll"
	File    "${appDir}\QtWebEngineProcess.exe"
	File    "${appDir}\UAVManage.exe"
	File    "${appDir}\UAVManage-UWB.exe"
	File /r "..\UAVManage\pythonapi"
	SetShellVarContext all
	CreateShortCut "$DESKTOP\���˻�������.lnk" "$INSTDIR\UAVManage.exe"
	CreateDirectory "$INSTDIR\Log"
	CreateDirectory "$INSTDIR\waypoint"
	
  StrCpy $File3DCount 0
  ;��ά����������
  ;IfFileExists "C:\WINDOWS\system32\x3daudio1_7.dll" 0 +2
  ;      StrCpy $File3DCount $File3DCount+1
  ;IfFileExists "C:\WINDOWS\system32\D3DCOMPILER_43.dll" 0 +2
  ;      StrCpy $File3DCount $File3DCount+2
  ;IfFileExists "C:\WINDOWS\system32\OPENGL32.dll" 0 +2
  ;      StrCpy $File3DCount $File3DCount+3
  ;IfFileExists "C:\WINDOWS\system32\VCRUNTIME140.dll" 0 +2
  ;      StrCpy $File3DCount $File3DCount+4
  ;${If} $File3DCount != "0+1+2+3+4"
  ;	     ;MessageBox MB_OK "��װ��ά������"
  ;      ExecWait "$INSTDIR\3D\Engine\Extras\Redist\en-us\UE4PrereqSetup_x64.exe"
  ;${EndIf}
  
  ;����ϵͳĿ¼�ض��򣬷���IfFileExists���Զ��ܵ�SysWOW64Ŀ¼Ѱ���ļ�
  ${DisableX64FSRedirection}
  ;������װ
  StrCpy $CP2102 0
  StrCpy $CH340 0
  IfFileExists "C:\WINDOWS\system32\drivers\silabser.sys" 0 +2
	StrCpy $CP2102 1
  IfFileExists "C:\WINDOWS\system32\drivers\CH343S64.SYS" 0 +2
	StrCpy $CH340 1
  ;MessageBox MB_OK "$CP2102$CH340"
  ${If} $CP2102 == "0"
	ExecWait "$INSTDIR\CP2102����\CP210xVCPInstaller_x64.exe"
  ${EndIf}
  ${If} $CH340 == "0"
	ExecWait "$INSTDIR\CH340����\CH343SER.exe"
  ${EndIf}
SectionEnd


Section -AdditionalIcons
  ;WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  ;��ʼ�˵�
  CreateDirectory "$SMPROGRAMS\���˻�������"
  CreateShortCut "$SMPROGRAMS\���˻�������\���˻�������.lnk" "$INSTDIR\UAVManage.exe"
  CreateShortCut "$SMPROGRAMS\���˻�������\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteRegDword HKLM "${PRODUCT_DIR_REGKEY}" "Installed" 1
  
  WriteUninstaller "$INSTDIR\uninst.exe"
  	; ע��
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\UAVManage.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\UAVManage.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  	; ����������
	;WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "SysSafeAddKey" "$INSTDIR\UAVManage.exe"
SectionEnd

#-- ���� NSIS �ű��༭�������� Function ���α�������� Section ����֮���д���Ա��ⰲװ�������δ��Ԥ֪�����⡣--#

; �����������
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} ""
!insertmacro MUI_FUNCTION_DESCRIPTION_END

/******************************
 *  �����ǰ�װ�����ж�ز���  *
 ******************************/
RequestExecutionLevel admin
Section Uninstall
	${DisableX64FSRedirection}
	SetRegView 64
	;Delete "$INSTDIR\${PRODUCT_NAME}.url"
	
	RMDir /r "$INSTDIR"
	SetShellVarContext all 
	Delete "$SMPROGRAMS\UAVManage\Uninstall.lnk"
	;Delete "$SMPROGRAMS\UAVManage\Website.lnk"
    Delete "$DESKTOP\���˻�������.lnk"
	Delete "C:\Users\Public\Desktop\���˻�������.lnk"
	RMDir /r "$SMPROGRAMS\���˻�������"

  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "SysSafeAddKey"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
  SetAutoClose true
  ;ɾ��ƾ֤

SectionEnd

#-- ���� NSIS �ű��༭�������� Function ���α�������� Section ����֮���д���Ա��ⰲװ�������δ��Ԥ֪�����⡣--#
Function .onInit
    ;��ֹ�����װ����ʵ��
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "JWBClient") i .r1 ?e'
	Pop $R0
	StrCmp $R0 0 +3
	MessageBox MB_OK|MB_ICONEXCLAMATION "�Ѿ���ʼ��װ��"
	Abort
	
	;��ֹ�ظ���װ����
	;ReadRegStr $0 HKLM '${PRODUCT_DIR_REGKEY}' ""
	;StrLen $1 $0
	;IntCmp $1 0 +3 +1 +1
	;MessageBox MB_OK|MB_USERICON '$(^Name) �Ѱ�װ�ڼ�����С��������°�װ����ж�����еİ�װ'
	;Quit
FunctionEnd

Function un.onInit  
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "��ȷʵҪ��ȫ�Ƴ� $(^Name) ���������е������" IDYES +2
  Abort
FunctionEnd

;ж�����
Function un.onUninstSuccess
  HideWindow
  ;MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) �ѳɹ��ش����ļ�����Ƴ���"
FunctionEnd

;��װ��ά������
Function Install3D
	ExecWait "$INSTDIR\3D\Engine\Extras\Redist\en-us\UE4PrereqSetup_x64.exe"
FunctionEnd

#��װ���
Function .onInstSuccess
  
FunctionEnd

;ѡ��װĿ¼ҳ
Function mulu
  ;���������ť
  ;FindWindow $0 "#32770" "" $HWNDPARENT
  ;GetDlgItem $0 $0 1001
  ;EnableWindow $0 0
  ;��ֹ�༭Ŀ¼����ֹ���޸�Ŀ¼���ж��ʱɾ���ļ���
  FindWindow $0 "#32770" "" $HWNDPARENT
  GetDlgItem $0 $0 1019
  EnableWindow $0 0
FunctionEnd