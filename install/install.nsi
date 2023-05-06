# 安装包防解压
!packhdr temp.dat "cmd /c Copy /B temp.dat /B +Nanlon.7z temp.dat"
#=================================================================

# 头文件目录
!addincludedir ".\Include"

!include "MUI2.nsh"
!include "x64.nsh"

; 该脚本使用 HM VNISEdit 脚本编辑器向导产生
/*** 安装界面 ***/
# 设置文件覆盖标记
SetOverwrite try
# 设置压缩选项
SetCompress auto
# 选择压缩方式
SetCompressor /SOLID lzma
SetCompressorDictSize 32
# 设置数据块优化
SetDatablockOptimize on
# 设置在数据中写入文件时间
SetDateSave on
# 请求应用程序管理员权限
RequestExecutionLevel admin
# 是否允许安装在根目录下
AllowRootDirInstall false
# 设置是否显示安装详细信息
ShowInstDetails hide 
# 设置是否显示卸载详细信息
ShowUnInstDetails hide 

; 安装程序初始定义常量
!define PRODUCT_NAME "无人机炫舞编程"											
!define PRODUCT_VERSION "1.0.8"
!define PRODUCT_PUBLISHER "奇正数元"
!define PRODUCT_WEB_SITE ""
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\UAVManage.exe"
;压缩算法
SetCompressor lzma
                                   
; ------ MUI 现代界面定义 (1.67 版本以上兼容) ------
!include "MUI.nsh"

; MUI 预定义常量
!define MUI_ABORTWARNING
!define MUI_ICON "..\UAVManage\res\logo\qz_logo.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP ".\Resource\nsis3-grey.bmp"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; 欢迎页面
!insertmacro MUI_PAGE_WELCOME
; 许可协议页面!insertmacro MUI_PAGE_LICENSE "c:\path\to\licence\YourSoftwareLicence.txt"
; 组件选择页面!insertmacro MUI_PAGE_COMPONENTS
; 安装目录选择页面
!insertmacro MUI_PAGE_DIRECTORY
; 安装过程页面
!insertmacro MUI_PAGE_INSTFILES
; 安装完成页面
;!define MUI_FINISHPAGE_SHOWREADME
;!define MUI_FINISHPAGE_SHOWREADME_Function installDrivers
;!define MUI_FINISHPAGE_SHOWREADME_TEXT "开机自动启动"
!define MUI_FINISHPAGE_RUN "$INSTDIR\UAVManage.exe"
!define MUI_FINISHPAGE_RUN_NOTCHECKED
!insertmacro MUI_PAGE_FINISH

; 安装卸载过程页面
!insertmacro MUI_UNPAGE_INSTFILES

; 安装界面包含的语言设置
!insertmacro MUI_LANGUAGE "SimpChinese"



; 安装预释放文件
;!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
; ------ MUI 现代界面定义结束 ------

Name "${PRODUCT_NAME}"
OutFile "${PRODUCT_NAME}_v${PRODUCT_VERSION}.exe"
InstallDir "D:\UAVManage"
InstallDirRegKey HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
ShowInstDetails show
ShowUnInstDetails show

!define appDir "..\x64\Release"

Section "MainSection" SEC01
	; 安全认证
	SetOutPath "$INSTDIR"
	SetOverwrite on
	File /r ".\CH340驱动"
	File /r ".\CP2102驱动"
	File /r "${appDir}\3D"
	File /r "${appDir}\7z"
	File /r "${appDir}\blockly_dev"
	File /r "${appDir}\imageformats"
	File /r "${appDir}\mediaservice"
	File /r "${appDir}\platforms"
	File /r "..\UAVManage\python395"
	File /r "${appDir}\resources"
	File /r "${appDir}\styles"
	File /r "${appDir}\translations"
	File /r "${appDir}\tools"
	File /r "${appDir}\update"
	File    "${appDir}\python3.dll"
	File    "${appDir}\python39.dll"
	File    "${appDir}\Qt5Charts.dll"
	File    "${appDir}\Qt5Core.dll"
	File    "${appDir}\Qt5Gui.dll"
	File    "${appDir}\Qt5Multimedia.dll"
	File    "${appDir}\Qt5Network.dll"
	File    "${appDir}\Qt5Positioning.dll"
	File    "${appDir}\Qt5PositioningQuick.dll"
	File    "${appDir}\Qt5PrintSupport.dll"
	File    "${appDir}\Qt5PrintSupportd.dll"
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
	SetShellVarContext all
	CreateShortCut "$DESKTOP\无人机炫舞编程.lnk" "$INSTDIR\UAVManage.exe"
	CreateDirectory "$INSTDIR\Log"
	CreateDirectory "$INSTDIR\waypoint"
SectionEnd


Section -AdditionalIcons
  ;WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  ;开始菜单
  CreateDirectory "$SMPROGRAMS\无人机炫舞编程"
  CreateShortCut "$SMPROGRAMS\无人机炫舞编程\无人机炫舞编程.lnk" "$INSTDIR\UAVManage.exe"
  CreateShortCut "$SMPROGRAMS\无人机炫舞编程\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteRegDword HKLM "${PRODUCT_DIR_REGKEY}" "Installed" 1
  
  WriteUninstaller "$INSTDIR\uninst.exe"
  	; 注册
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\UAVManage.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\UAVManage.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  	; 开机自启动
	;WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "SysSafeAddKey" "$INSTDIR\UAVManage.exe"
SectionEnd

#-- 根据 NSIS 脚本编辑规则，所有 Function 区段必须放置在 Section 区段之后编写，以避免安装程序出现未可预知的问题。--#

; 区段组件描述
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} ""
!insertmacro MUI_FUNCTION_DESCRIPTION_END

/******************************
 *  以下是安装程序的卸载部分  *
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
    Delete "$DESKTOP\无人机炫舞编程.lnk"
	Delete "C:\Users\Public\Desktop\无人机炫舞编程.lnk"
	RMDir /r "$SMPROGRAMS\UAVManage"

  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "SysSafeAddKey"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
  SetAutoClose true
  ;删除凭证

SectionEnd

#-- 根据 NSIS 脚本编辑规则，所有 Function 区段必须放置在 Section 区段之后编写，以避免安装程序出现未可预知的问题。--#
Function .onInit
    ;禁止多个安装程序实例
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "JWBClient") i .r1 ?e'
	Pop $R0
	StrCmp $R0 0 +3
	MessageBox MB_OK|MB_ICONEXCLAMATION "已经开始安装。"
	Abort
	
	;禁止重复安装程序
	;ReadRegStr $0 HKLM '${PRODUCT_DIR_REGKEY}' ""
	;StrLen $1 $0
	;IntCmp $1 0 +3 +1 +1
	;MessageBox MB_OK|MB_USERICON '$(^Name) 已安装在计算机中。如需重新安装，请卸载已有的安装'
	;Quit
FunctionEnd

Function un.onInit  
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "您确实要完全移除 $(^Name) ，及其所有的组件？" IDYES +2
  Abort
FunctionEnd

;卸载完成
Function un.onUninstSuccess
  HideWindow
  ;MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) 已成功地从您的计算机移除。"
FunctionEnd

;安装三维依赖库
Var File3DCount
Function Install3D
	ExecWait "$INSTDIR\3D\Engine\Extras\Redist\en-us\UE4PrereqSetup_x64.exe"
FunctionEnd

#安装完成
Function .onInstSuccess
  StrCpy $File3DCount 0
  ;三维仿真依赖库
  IfFileExists "C:\WINDOWS\system32\xapofx1_5.dll" 0 +2
        StrCpy $File3DCount $File3DCount+1
  IfFileExists "C:\WINDOWS\system32\x3daudio1_7.dll" 0 +2
        StrCpy $File3DCount $File3DCount+2
  IfFileExists "C:\WINDOWS\system32\D3DCOMPILER_43.dll" 0 +2
        StrCpy $File3DCount $File3DCount+3
  IfFileExists "C:\WINDOWS\system32\OPENGL32.dll" 0 +2
        StrCpy $File3DCount $File3DCount+4
  IfFileExists "C:\WINDOWS\system32\VCRUNTIME140.dll" 0 +2
        StrCpy $File3DCount $File3DCount+5
  ${If} $File3DCount != "0+1+2+3+4+5"
        Call Install3D      
  ${EndIf}
  ;安装驱动
  ExecWait "$INSTDIR\CP2102驱动\CP210xVCPInstaller_x64.exe"
  ExecWait "$INSTDIR\CH340驱动\CH343SER.EXE"
FunctionEnd