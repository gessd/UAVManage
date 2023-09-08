#include "uavmanage.h"
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QDir>
#include <QTime>
#include <QTimer>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QTextCodec>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QStandardPaths>
#include <QMimeData>
#include "definesetting.h"
#include "tinyxml2/tinyxml2.h"
#include "messagelistdialog.h"
#include "placeinfodialog.h"
#include "musicplayer.h"
#include "paramreadwrite.h"
#include "spaceparam.h"
#include "waitingwidget.h"
#include "qxtglobalshortcut.h"
#include "historymessage.h"
#include "registerdialog.h"
#include "stopflydialog.h"
#include "firmwaredialog.h"
#include "managertopwidget.h"
#include "mytooltip.h"
#include "define3d.h"
#include "threadpython.h"

UAVManage::UAVManage(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	m_pSocketServer = nullptr;
	m_pWebBlocklySocket = nullptr;
	m_pDeviceManage = nullptr;
	m_p3DProcess = nullptr;
	m_pBackgrounMask = nullptr;
	m_pToopTip = nullptr;
	m_pMenuWidget = nullptr;
	//注册事件过滤器，处理快捷键事件
	installEventFilter(this);
	//setWindowFlags(Qt::FramelessWindowHint);
	MessageListDialog::getInstance()->setParent(this);
	m_pHistory = new HistoryMessage(this);
	connect(MessageListDialog::getInstance(), SIGNAL(sigMessage(QString, _Messagelevel, bool)), m_pHistory, SLOT(onMessageData(QString, _Messagelevel, bool)));
	//程序初始化
	connect(ui.webEngineView, SIGNAL(loadProgress(int)), this, SLOT(onWebLoadProgress(int)));
	connect(ui.webEngineView, SIGNAL(loadFinished(bool)), this, SLOT(onWebLoadFinished(bool)));
	//初始化web
	m_pSocketServer = new QWebSocketServer(QStringLiteral("Socket Server"), QWebSocketServer::NonSecureMode, this);
	//m_pSocketServer->listen(QHostAddress::Any, _WebSocketPort_);
	connect(m_pSocketServer, SIGNAL(newConnection()), this, SLOT(onSocketNewConnection()));

	m_pTopWidget = new ManagerTopWidget(this);
	m_pTopWidget->close();
	connect(m_pTopWidget, &ManagerTopWidget::sigPrepareWidget, this, &UAVManage::onPrepareWidget);
	//增加设备列表
	m_pDeviceManage = new DeviceManage(this);
	m_pDeviceManage->setEnabled(false);
	ui.layoutManger->addWidget(m_pDeviceManage);
	connect(m_pDeviceManage, &DeviceManage::currentDeviceNameChanged, this, &UAVManage::onCurrentDeviceNameChanged);
	connect(m_pDeviceManage, &DeviceManage::deviceAddFinished, this, &UAVManage::onDeviceAdd);
	connect(m_pDeviceManage, &DeviceManage::deviceRemoveFinished, this, &UAVManage::onDeviceRemove);
	connect(m_pDeviceManage, &DeviceManage::deviceRenameFinished, this, &UAVManage::onDeviceRename);
	connect(m_pDeviceManage, &DeviceManage::deviceResetIp, this, &UAVManage::onDeviceResetIp);
	connect(m_pDeviceManage, &DeviceManage::deviceResetLocation, this, &UAVManage::onDeviceResetLocation);
	connect(m_pDeviceManage, &DeviceManage::sigWaypointFinished, this, &UAVManage::onWaypointFinished);
	//因槽函数中有exec阻塞，使用QueuedConnection不等待槽函数执行过程
	connect(m_pDeviceManage, &DeviceManage::sigTakeoffFinished, this, &UAVManage::onDeviceTakeoffFinished, Qt::QueuedConnection);
	connect(m_pDeviceManage, &DeviceManage::sig3DDialogStatus, this, &UAVManage::on3DDialogStauts);
	connect(m_pDeviceManage, &DeviceManage::sigStart3D, this, &UAVManage::onStart3DDialog);
	connect(m_pDeviceManage, &DeviceManage::sigPrepareWidget, this, &UAVManage::onPrepareWidget);
	connect(m_pDeviceManage, &DeviceManage::sigBlockFlicker, this, &UAVManage::onBlockFlicker);

	m_pAbout = new AboutDialog(this);
	m_pMusicPlayer = new MusicPlayer(this);
	m_pMusicPlayer->setEnabled(false);
	connect(m_pMusicPlayer, &MusicPlayer::sigUpdateMusic, this, &UAVManage::onUpdateMusic);
	connect(m_pMusicPlayer, &MusicPlayer::sigMsuicTime, this, &UAVManage::onCurrentMusicTime);
	connect(m_pMusicPlayer, &MusicPlayer::playeState, this, &UAVManage::onCurrentPlayeState);
	connect(m_pMusicPlayer, &MusicPlayer::updateMusicWaveFinished, this, &UAVManage::onMusicWaveFinished);
	connect(m_pMusicPlayer, &MusicPlayer::sigUpdateMusicTime, m_pDeviceManage, &DeviceManage::onUpdateMusicMaxTime);
	ui.horizontalLayoutSound->addWidget(m_pMusicPlayer);
	
	connect(QZAPI::Instance(), SIGNAL(sigBlockFlicker(QString)), this, SLOT(onBlockFlicker(QString)));

	connect(&m_timerLogFile, &QTimer::timeout, this, &UAVManage::onTimerLogFile);
	m_timerLogFile.start(10 * 60 * 1000);

	m_pToopTip = new MyTooltip(this);
	initMenu();
}

UAVManage::~UAVManage()
{
	if (m_pWebBlocklySocket) {
		m_pWebBlocklySocket->disconnected();
	}
	if (m_p3DProcess) {
		m_p3DProcess->close();
		m_p3DProcess->kill();
		m_p3DProcess->deleteLater();
		m_p3DProcess = nullptr;
	}
	if (m_pMusicPlayer) {
		delete m_pMusicPlayer;
		m_pMusicPlayer = nullptr;
	}
	if (m_pSocketServer) {
		m_pSocketServer->close();
		delete m_pSocketServer;
		m_pSocketServer = nullptr;
	}
	if (m_pDeviceManage) {
		m_pDeviceManage->clearDevice();
		delete m_pDeviceManage;
		m_pDeviceManage = nullptr;
	}
	if (m_pAbout) {
		m_pAbout->close();
		m_pAbout->deleteLater();
		m_pAbout = nullptr;
	}
}

void UAVManage::initMenu()
{
	static bool bInit = false;
	if (bInit) return;
	bInit = true;
	//添加自定义菜单
	ui.menuBar->setFixedHeight(30);
	m_pMenuWidget = new QWidget(ui.menuBar);
	QHBoxLayout* pMenuLayout = new QHBoxLayout(m_pMenuWidget);
	pMenuLayout->setSpacing(10);
	pMenuLayout->setContentsMargins(10, 0, 10, 0);
	m_pMenuWidget->setLayout(pMenuLayout);
	m_pMenuWidget->setGeometry(0, 0, ui.menuBar->width(), ui.menuBar->height());
	//QLabel* pLableIcon = new QLabel(this);
	//pLableIcon->setFixedSize(22, 22);
	//pLableIcon->setPixmap(QPixmap(":/res/images/logo.png"));
	//pLableIcon->setScaledContents(true);
	//pMenuLayout->addWidget(pLableIcon);

	QMenu* pIconMenu = new QMenu(tr("奇正数元"));
	pIconMenu->setIcon(QIcon(":/res/logo/qz_logo.ico"));
	pIconMenu->setWindowFlags(pIconMenu->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
	pIconMenu->setAttribute(Qt::WA_TranslucentBackground);
	QAction* pQssAction = new QAction("更新样式");
	QAction* pMessage = new QAction("消息窗口");
	pIconMenu->addAction(pQssAction);
	pIconMenu->addAction(pMessage);
	connect(pQssAction, &QAction::triggered, [this]() { updateStyle(); });
	connect(pMessage, &QAction::triggered, [this]() {
		_ShowErrorMessage(tr("这是错误消息这是错误消息这是错误消息这是错误消息这是错误消息这是错误消息这是错误消息"));
		_ShowWarningMessage(tr("警告消息警告消息警告消息警告消息警告消息警告消息警告消息警告消息"));
		_ShowInfoMessage(tr("正常提示消息正常提示消息正常提示消息正常提示消息正常提示消息正常提示消息正常提示消息正常提示消息正常提示消息"));
		});
	//项目管理菜单
	QMenu* pProjectMenu = new QMenu(tr("项目"));
	pProjectMenu->setWindowFlags(pProjectMenu->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
	pProjectMenu->setAttribute(Qt::WA_TranslucentBackground);
	QAction* pActionNew = new QAction(QIcon(":/res/menu/P01_file_new_btn_nor.png"), tr("新建"));
	QAction* pActionOpen = new QAction(QIcon(":/res/menu/P01_file_open_btn_nor.png"), tr("打开"));
	QAction* pActionSaveas = new QAction(QIcon(":/res/menu/P01_file_saveus_btn_nor.png"), tr("另存为"));
	QAction* pActionClose = new QAction(QIcon(":/res/images/closeproject.png"), tr("关闭"));
	m_pActionAttribute = new QAction(QIcon(":/res/menu/P01_file_saveus_btn_nor.png"), tr("属性"));
	pProjectMenu->addAction(pActionNew);
	pProjectMenu->addAction(pActionOpen);
	pProjectMenu->addAction(pActionSaveas);
	pProjectMenu->addAction(pActionClose);
	pProjectMenu->addSeparator();
	pProjectMenu->addAction(m_pActionAttribute);
	m_pActionAttribute->setEnabled(false);
	connect(pActionNew, &QAction::triggered, [this]() { onNewProject(); });
	connect(pActionOpen, &QAction::triggered, [this]() {
		QString qstrFile = QFileDialog::getOpenFileName(this, tr("项目名"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "File(*.qz)");
		if (qstrFile.isEmpty()) return;
		onOpenProject(qstrFile);
		});
	connect(pActionSaveas, &QAction::triggered, [this]() { onSaveasProject(); });
	connect(pActionClose, &QAction::triggered, [this]() { onCloseProject(); });
	connect(m_pActionAttribute, &QAction::triggered, [this]() { onProjectAttribute(); });

	//三维预览进程
	m_p3DProcess = new QProcess(this);
	//起飞准备菜单
	QMenu* pMenuFlyPrepare = new QMenu(tr("起飞准备"));
	pMenuFlyPrepare->setWindowFlags(pMenuFlyPrepare->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
	pMenuFlyPrepare->setAttribute(Qt::WA_TranslucentBackground);

	pMenuLayout->addWidget(initMenuButton(m_pMenuWidget, tr("项目"), ":/res/menu/P01_file_open_btn_cli.png", ":/res/menu/P01_file_open_btn_cli.png", pProjectMenu));
	QMenu* pActionHelp = new QMenu(tr("帮助"));
	pActionHelp->setWindowFlags(pMenuFlyPrepare->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
	pActionHelp->setAttribute(Qt::WA_TranslucentBackground);
	QAction* pActionAbout = new QAction(QIcon(":/res/menu/P02_help_about_btn_new_ic.png"), tr("版本"));
	pActionHelp->addAction(pActionAbout);
	QAction* pActionFirmware = new QAction(QIcon(":/res/menu/firmware.png"), tr("固件"));
	pActionHelp->addAction(pActionFirmware);
	
	pMenuLayout->addWidget(initMenuButton(m_pMenuWidget, tr("帮助"), ":/res/menu/P02_help_about_page_ic.png", ":/res/menu/P02_help_about_page_ic.png", pActionHelp));
	connect(pActionAbout, &QAction::triggered, [this]() {m_pAbout->exec(); });
	connect(pActionFirmware, &QAction::triggered, [this]() {
		m_pDeviceManage->showFirmwareDialog();
		});
	m_pRegister = new RegisterDialog(this);
	QAction* pActionReg = new QAction(QIcon(":/res/menu/authority.png"), tr("授权"));
	connect(pActionReg, &QAction::triggered, [this]() {m_pRegister->exec(); });
	pActionHelp->addAction(pActionReg);
	pMenuLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

	QString text = tr("切换UWB基站模式");
#ifdef _UseUWBData_
	text = tr("切换WIFI网络模式");
#endif
	QAction* pActionModeSwitch = new QAction(QIcon(""), text);
	pActionHelp->addAction(pActionModeSwitch);
	connect(pActionModeSwitch, &QAction::triggered, [this]() {
		QMessageBox::StandardButton button = QMessageBox::question(this, tr("询问"), tr("切换模式需要重新启动软件，是否切换？"));
		if (QMessageBox::StandardButton::Yes != button)  return;
		QProcess* process = new QProcess;
#ifdef _UseUWBData_
		ParamReadWrite::writeParam("UWB", false);
		process->start("UAVManage.exe");
#else
		ParamReadWrite::writeParam("UWB", true);
		process->start("UAVManage-UWB.exe");
#endif
		qApp->quit();
		});

	m_pStopDialog = new StopFlyDialog(this);
	m_pStopDialog->close();
	connect(m_pStopDialog, &StopFlyDialog::sigFlyControl, [this](bool stop) {
		if (stop)m_pDeviceManage->allDeviceControl(_DeviceQuickStop);
		else m_pDeviceManage->allDeviceControl(_DeviceLandLocal);
		});

	//无边框时程序增加最大化最小化按钮
	pMenuLayout->addItem(new QSpacerItem(40, 20));
	QToolButton* pBtnMin = new QToolButton(this);
	pBtnMin->setFixedSize(m_pMenuWidget->height(), m_pMenuWidget->height());
	pBtnMin->setIconSize(QSize(12, 12));
	pBtnMin->setIcon(QIcon(":/res/images/min.png"));
	pMenuLayout->addWidget(pBtnMin);
	m_pBtnMax = new QToolButton(this);
	m_pBtnMax->setFixedSize(m_pMenuWidget->height(), m_pMenuWidget->height());
	m_pBtnMax->setIconSize(QSize(12, 12));
	m_pBtnMax->setIcon(QIcon(":/res/images/max.png"));
	pMenuLayout->addWidget(m_pBtnMax);
	QToolButton* pBtnClose = new QToolButton(this);
	pBtnClose->setFixedSize(m_pMenuWidget->height(), m_pMenuWidget->height());
	pBtnClose->setIconSize(QSize(12, 12));
	pBtnClose->setIcon(QIcon(":/res/images/close.png"));
	pMenuLayout->addWidget(pBtnClose);
	m_pBtnMax->setVisible(false);
	pBtnMin->setVisible(false);
	pBtnClose->setVisible(false);
	connect(pBtnMin, &QAbstractButton::clicked, this, &UAVManage::showMinimized);
	connect(pBtnClose, &QAbstractButton::clicked, this, &UAVManage::close);
	connect(m_pBtnMax, &QAbstractButton::clicked, [this]() {
		if (Qt::WindowNoState == windowState()) {
			showMaximized();
			QDesktopWidget* desktopWidget = QApplication::desktop();
			QRect screenRect = desktopWidget->screenGeometry();
			//不能完全全屏，需要减去任务栏位置
			setFixedSize(screenRect.width(), screenRect.height() - 40);
			move(0, 0);
			m_pBtnMax->setIcon(QIcon(":/res/images/normal.png"));
		}
		else {
			showNormal();
			m_pBtnMax->setIcon(QIcon(":/res/images/max.png"));
		}
		});
}

void UAVManage::updateStyle()
{
	QStyle* style = QApplication::style();
    QIcon icon = style->standardIcon(QStyle::SP_ComputerIcon);
	QFile file("E:/fly/UAVManage/UAVManage/res/qss/style.qss");
	if (file.open(QIODevice::ReadOnly)) {
		qApp->setStyleSheet(file.readAll());
		file.close();
	}
}

void UAVManage::loadWeb()
{
	QString qstrUrl = "file:///" + QApplication::applicationDirPath() + "/blockly_dev/blockly_dev/code/index.html";
	qInfo() << "打开编程区" << qstrUrl;
	ui.webEngineView->load(QUrl(qstrUrl));
}

QString UAVManage::getCurrentBlocklyFile()
{
	if(m_qstrCurrentProjectFile.isEmpty() || !m_pDeviceManage) return QString();
	QString name = m_pDeviceManage->getCurrentDeviceName();
	if (name.isEmpty()) return QString();
	QFileInfo info(m_qstrCurrentProjectFile);
	QString path = info.path();
	return QString("%1%2%3.blockly").arg(path).arg(_ProjectDirName_).arg(name);
}

QString UAVManage::getCurrentPythonFile(bool manual)
{
	if (m_qstrCurrentProjectFile.isEmpty() || !m_pDeviceManage) return QString();
	QString name = m_pDeviceManage->getCurrentDeviceName();
	if (name.isEmpty()) return QString();
	QFileInfo info(m_qstrCurrentProjectFile);
	QString path = info.path();
	//mp手动编写的python文件
	if(manual) return QString("%1%2%3.mp").arg(path).arg(_ProjectDirName_).arg(name);
	return QString("%1%2%3.py").arg(path).arg(_ProjectDirName_).arg(name);
}

bool UAVManage::initGlobalShortcut(QString shortcutKey)
{
	qInfo() << "注册全局快捷键" << shortcutKey;
	QxtGlobalShortcut* pShortcut = new QxtGlobalShortcut(this);
	if (false == pShortcut->setShortcut(QKeySequence(shortcutKey))) {
		qInfo() << "快捷键已占用" << shortcutKey;
		_ShowErrorMessage("急停快捷键(Ctrl+空格)已被其他程序占用，请解除后重新启动程序");
		return false;
	}
	QObject::connect(pShortcut, &QxtGlobalShortcut::activated, [&]() {
		qDebug() << "全局快捷键控制";
		m_pDeviceManage->allDeviceControl(_DeviceQuickStop);
		});
	qInfo() << "全局快捷键注册成功";
	return true;
}

void UAVManage::onNewProject()
{
	SpaceParam space(true, this);
	if (QDialog::Accepted != space.exec())return;
	onCloseProject();
	//场地大小
	unsigned int x = space.getSpaceX();
	unsigned int y = space.getSpaceY();
	QString qstrName = space.getProjectPath();
	//选择新建路径
	//QString qstrName = QFileDialog::getSaveFileName(this, tr("项目名"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), tr("File(*.qz)"));
	//if (qstrName.isEmpty()) return;
	qInfo() << "开始新建项目" << qstrName;
	//新建项目文件夹
	QFileInfo info(qstrName);
	QString qstrDir = info.path() + "/" + info.baseName();
	QString qstrFile = qstrDir + "/" + info.fileName() + "." + _ProjectSuffix;
	QDir dir;
	if (!dir.mkdir(qstrDir)) {
		_ShowErrorMessage(info.baseName() + tr("项目创建失败"));
		return;
	}
	dir.mkdir(qstrDir + _ProjectDirName_);
	//新建项目文件并写入初始化内容
	if (!newProjectFile(qstrFile, x, y)) {
		dir.rmdir(qstrDir);
		_ShowErrorMessage(info.baseName() + tr("创建项目文件失败"));
		return;
	}
	_ShowInfoMessage(info.baseName()+tr("新建项目完成"));
	onOpenProject(qstrFile);
}

void UAVManage::onOpenProject(QString qstrFile)
{
	onCloseProject();
	qInfo() << "打开工程" << qstrFile;
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string filename = code->fromUnicode(qstrFile).data();
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) {
		_ShowErrorMessage(tr("打开工程文件失败，不是有效工程文件"));
		return;
	}
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root) {
		_ShowErrorMessage(tr("读取工程文件失败，工程文件不完整"));
		return;
	}
	tinyxml2::XMLElement* place = root->FirstChildElement(_ElementPlace_);
	if (!place) {
		_ShowErrorMessage(tr("无法打开，缺少关键数据"));
		return;
	}
	//读取场地大小
	unsigned int x = place->IntAttribute(_AttributeX_);
	unsigned int y = place->IntAttribute(_AttributeY_);
	m_pDeviceManage->setSpaceSize(x, y);
	if (m_pWebBlocklySocket) {
		qInfo() << "发送场地范围到编程区";
		QPoint space = m_pDeviceManage->getSpaceSize();
		QJsonObject jsonObj;
		jsonObj.insert(_WMID, _WIDSet);
		jsonObj.insert("x", space.x());
		jsonObj.insert("y", space.y());
		jsonObj.insert("z", _MaxFlyHeight_);
		QJsonDocument jsonDoc(jsonObj);
		m_pWebBlocklySocket->sendTextMessage(jsonDoc.toJson());
	}
	QFileInfo infoProject(qstrFile);
	QString qstrMusicFilePath = infoProject.path() + _ProjectDirName_ + place->Attribute(_ElementMusic_);
	QString qstrCurrnetName = place->Attribute(_AttributeName_);
	tinyxml2::XMLElement* device = root->FirstChildElement(_ElementDevice_);
	if (!device) {
		_ShowErrorMessage(tr("工程中没有添加设备"));
	}
	m_qstrCurrentProjectFile = qstrFile;
	m_pDeviceManage->setCrrentProject(m_qstrCurrentProjectFile);
	int nTagTemp = 1;
	bool bUpdateXML = false;
	while (device){
		//遍历无人机属性
		QString devicename = device->Attribute(_AttributeName_);
#ifdef _UseUWBData_
		QString ip = device->Attribute(_AttributeTag_);
		if (ip.isEmpty()) {
			//标签为空说名是以前没有写入标签时建立的项目
			ip = QString::number(nTagTemp);
			device->SetAttribute(_AttributeTag_, ip.toUtf8().data());
			nTagTemp++;
			bUpdateXML = true;
		}
#else
		QString ip = device->Attribute(_AttributeIP_);
#endif
		int x = device->IntAttribute(_AttributeX_);
		int y = device->IntAttribute(_AttributeY_);
		device = device->NextSiblingElement(_ElementDevice_);
		if (m_pDeviceManage) {
			QString error = m_pDeviceManage->addDevice(devicename, ip, x, y, false);
			if (!error.isEmpty()) {
				_ShowErrorMessage(tr("无法使用设备") + devicename + error);
			}
		}
	}
	if(bUpdateXML) error = doc.SaveFile(filename.c_str());
	_ShowInfoMessage(tr("打开工程完成"));
	QFileInfo info(m_qstrCurrentProjectFile);
	QString name = info.baseName();
	m_pDeviceManage->setCurrentDevice(qstrCurrnetName);
	m_pMusicPlayer->updateLoadMusic(qstrMusicFilePath);
	m_pDeviceManage->setEnabled(true);
	m_pMusicPlayer->setEnabled(true);
	ParamReadWrite::writeParam(_Path_, m_qstrCurrentProjectFile);
	m_pActionAttribute->setEnabled(true);
}

//拷贝文件夹
bool copyDirectoryFiles(const QString& fromDir, const QString& toDir, bool coverFileIfExist)
{
	QDir sourceDir(fromDir);
	QDir targetDir(toDir);
	if (!targetDir.exists()) {    /**< 如果目标目录不存在，则进行创建 */
		if (!targetDir.mkdir(targetDir.absolutePath()))
			return false;
	}

	QFileInfoList fileInfoList = sourceDir.entryInfoList();
	foreach(QFileInfo fileInfo, fileInfoList) {
		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;

		if (fileInfo.isDir()) {    /**< 当为目录时，递归的进行copy */
			if (!copyDirectoryFiles(fileInfo.filePath(),
				targetDir.filePath(fileInfo.fileName()),
				coverFileIfExist))
				return false;
		}
		else {            /**< 当允许覆盖操作时，将旧文件进行删除操作 */
			if (coverFileIfExist && targetDir.exists(fileInfo.fileName())) {
				targetDir.remove(fileInfo.fileName());
			}

			/// 进行文件copy
			if (!QFile::copy(fileInfo.filePath(),
				targetDir.filePath(fileInfo.fileName()))) {
				return false;
			}
		}
	}
	return true;
}

void UAVManage::onSaveasProject()
{
	if (m_qstrCurrentProjectFile.isEmpty()) return;
	//复制工程到新文件夹并重命名
	QString qstrName = QFileDialog::getSaveFileName(this, tr("项目名"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), "File(*.qz)");
	if (qstrName.isEmpty()) return;
	QFileInfo info(qstrName);
	QString qstrPath = info.path();
	info.setFile(m_qstrCurrentProjectFile);
	QString qstrFromPath = info.path()+ _ProjectDirName_;
	if (QFile::exists(qstrName)) {
		QFile::remove(qstrName);
	}else {
		QFileInfo info(qstrName);
		qstrPath = info.path() + "/" + info.baseName();
		qstrName = qstrPath + "/" + info.fileName();
		//新建项目文件夹
		QDir dir;
		if (!dir.mkdir(qstrPath)) return;
	}
	//复制积木块文件到新文件夹
	copyDirectoryFiles(qstrFromPath, qstrPath + _ProjectDirName_, true);
	bool bcp = QFile::copy(m_qstrCurrentProjectFile, qstrName);
	m_qstrCurrentProjectFile = qstrName;
	m_pDeviceManage->setCrrentProject(m_qstrCurrentProjectFile);
	ParamReadWrite::writeParam(_Path_, m_qstrCurrentProjectFile);
}

void UAVManage::onCloseProject()
{
	//清空项目信息
	m_pMusicPlayer->clearSound();
	if (m_pDeviceManage) m_pDeviceManage->clearDevice();
	onWebClear();
	m_pActionAttribute->setEnabled(false);
	m_pDeviceManage->setEnabled(false);
	m_pMusicPlayer->setEnabled(false);
	//先清空数据
	if (false == m_qstrCurrentProjectFile.isEmpty()) {
		QFileInfo info(m_qstrCurrentProjectFile);
		_ShowInfoMessage(tr("关闭工程") + info.baseName());
	}
	m_qstrCurrentProjectFile.clear();
	m_pDeviceManage->setCrrentProject(m_qstrCurrentProjectFile);
	ParamReadWrite::writeParam(_Path_, "");
}

void UAVManage::onWebClear()
{
	if (!m_pWebBlocklySocket) return;
	QJsonObject jsonObj;
	jsonObj.insert(_WMID, _WIDClear);
	QJsonDocument doc(jsonObj);
	m_pWebBlocklySocket->sendTextMessage(doc.toJson());
}

void UAVManage::showEvent(QShowEvent* event)
{
	qInfo() << "显示主窗口";
	MessageListDialog::getInstance()->setParent(this);
	if (!m_pWebBlocklySocket) {
		bool temp = m_pSocketServer->listen(QHostAddress::Any, _WebSocketPort_);
		if (false == temp) {
			emit sigWindowFinished();
			QString error = QString::number(_WebSocketPort_) + tr("电脑端口被占，无法正常使用");
			qWarning() << error;
			QMessageBox::warning(this, tr("错误"), error);
			qApp->quit();
			return;
		}
		temp = m_pDeviceManage->start3DTcp();
		if (false == temp) {
			emit sigWindowFinished();
			QString error = QString::number(_TcpPort_) + tr("电脑端口被占，无法正常使用");
			qWarning() << error;
			QMessageBox::warning(this, tr("错误"), error);
			qApp->quit();
			return;
		}
		initGlobalShortcut("Ctrl+Space");
		loadWeb();
	}
	m_pHistory->resetWidget();	
}

void UAVManage::closeEvent(QCloseEvent* event)
{
	QMessageBox::StandardButton button = QMessageBox::question(this, "关闭", "软件关闭后需要重新进行三维仿真、基站标定、上传舞步等操作，确认关闭软件吗？");
	if (QMessageBox::StandardButton::Yes != button) {
		event->ignore();
		return;
	}
	MessageListDialog::getInstance()->exitDialog();
}

void UAVManage::resizeEvent(QResizeEvent* event)
{
	if (m_pMenuWidget) m_pMenuWidget->setGeometry(0, 0, ui.menuBar->width(), ui.menuBar->height());
	MessageListDialog::getInstance()->move((width() - MessageListDialog::getInstance()->width()) / 2, 0);
	m_pHistory->resetWidget();
	if(!m_pTopWidget->isHidden()) m_pTopWidget->setGeometry(0, 0, width(), height());
}

bool UAVManage::eventFilter(QObject* watched, QEvent* event)
{
	//鼠标控制窗口移动
	static bool bMove = false;
	static QPoint reltvPos;
	if (this == watched || ui.menuBar == watched) {
		if (QEvent::MouseButtonPress == event->type()) {
			QMouseEvent* mouse = dynamic_cast<QMouseEvent*>(event);
			reltvPos = mouse->pos();
			//bMove = true;
		}
		else if (QEvent::MouseButtonRelease == event->type()) {
			bMove = false;
		}
		else if (QEvent::MouseMove == event->type()) {
			if (bMove) {
				QMouseEvent* mouse = dynamic_cast<QMouseEvent*>(event);
				move(mouse->globalPos() - reltvPos);
			}
		}
	}

	//自定义Tip用于永久显示
	static QPoint mouseGloblePos;
	if (event->type() == QEvent::MouseMove) {
		mouseGloblePos = static_cast<QMouseEvent*>(event)->globalPos();
	}
	if (QEvent::ToolTip == event->type()) {
		QWidget* pWidget = dynamic_cast<QWidget*>(watched);
		if (pWidget) {
			QString qstrText = pWidget->toolTip();
			if (!qstrText.isEmpty()) {
				m_pToopTip->setText(qstrText);
				QHelpEvent HE(QEvent::Type(MyTooltip::MyToolTipEvent), QPoint(), mouseGloblePos);
				QApplication::sendEvent(m_pToopTip, &HE);
				return true;
			}
		}
	}
	if (event->type() == QEvent::Leave) {
		m_pToopTip->onHide();
	}

	//使用快捷键控制
	if (event->type() == QEvent::KeyRelease) {
		QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
		if (keyEvent) {
			//增加快捷键控制急停
			if (keyEvent->modifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_Space) {
				qInfo() << "快捷键控制急停";
				m_pDeviceManage->allDeviceControl(_DeviceQuickStop);
			}
		}
	}
	return __super::eventFilter(watched, event);
}

void UAVManage::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasUrls())
		event->acceptProposedAction();
}

void UAVManage::dropEvent(QDropEvent* event)
{
	QList<QUrl> urls = event->mimeData()->urls();
	qDebug() << "拖放文件" << urls;
	foreach(QUrl url, urls) {
		QString path = url.path().remove(0, 1);
		qDebug() << path;
		QFileInfo info(path);
		if (info.isFile()) {
			QString suffix = info.suffix();
			if (_ProjectSuffix == suffix) {
				onOpenProject(path);
				return;
			}
			else if ("mp3" == suffix || "wav" == suffix) {
				onUpdateMusic(path);
				return;
			}
		}
		else if (info.isDir()) {
			QDir dir(path);
			QFileInfoList fileInfoList = dir.entryInfoList();
			foreach(QFileInfo fileInfo, fileInfoList) {
				if (fileInfo.fileName() == "." || fileInfo.fileName() == "..") continue;
				if (_ProjectSuffix == fileInfo.suffix()) {
					onOpenProject(fileInfo.filePath());
					return;
				}
			}
		}
	}
}

void UAVManage::onWebLoadProgress(int progress)
{
	//网页加载进度
}

void UAVManage::onWebLoadFinished(bool finished)
{
	//网页加载完成
	qInfo() << "编程区加载完成";
}

void UAVManage::onSocketNewConnection()
{
	//不能同时打开两个软件，否则端口占用无法使用
	//只记录一个web连接，防止通过浏览器访问blockly
	QWebSocket* pSocket = m_pSocketServer->nextPendingConnection();
	qDebug() << "blockly新连接" << pSocket->peerAddress() << pSocket->peerPort();
	if (m_pWebBlocklySocket) {
		if (pSocket) pSocket->disconnected();
		return;
	}
	qInfo() << "已建立编程区域连接";
	m_pWebBlocklySocket = pSocket;
	connect(m_pWebBlocklySocket, SIGNAL(textMessageReceived(QString)), this, SLOT(onSocketTextMessageReceived(QString)));
	connect(m_pWebBlocklySocket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
	//打开上次记录的项目
	QString qstrPath = ParamReadWrite::readParam(_Path_).toString();
	if (false == qstrPath.isEmpty() && QFile::exists(qstrPath)) {
		onOpenProject(qstrPath);
	}
	QTimer::singleShot(1000, [this]() { 
		emit sigWindowFinished(); 
		//判断是否注册授权
		if (false == m_pRegister->isRegister()) m_pRegister->exec();
		});
}

void UAVManage::onSocketTextMessageReceived(QString message)
{
	//防止数据大量重复处理
	static QString qstrLastWebMessage = "";
	if (qstrLastWebMessage == message) return;
	qstrLastWebMessage = message;
	QJsonParseError jsonError;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8(), &jsonError);
	if (QJsonParseError::NoError != jsonError.error || jsonDoc.isEmpty() || !jsonDoc.isObject()) {
		return;
	}
	//使用python方式传送blockly中的数据
	QJsonObject jsonObj = jsonDoc.object();
	int id = jsonObj.value(_WMID).toInt();
	if (_WIDUpdate != id && _WIDManual != id) return;
	QString xml = jsonObj.value(_name2str(xml)).toString();
	QString python = jsonObj.value(_name2str(python)).toString();
	QString blocklyFileName = getCurrentBlocklyFile();
	QString pythonFileName = getCurrentPythonFile();	
	if (blocklyFileName.isEmpty() || pythonFileName.isEmpty()) return;
	if (_WIDUpdate == id) {
		QFile blocklyFile(blocklyFileName);
		qInfo() << "积木块区域变动，自动保存文件" << blocklyFileName;
		if (blocklyFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			blocklyFile.write(xml.toUtf8());
			blocklyFile.close();
		}
	} else if (_WIDManual == id) {
		pythonFileName = getCurrentPythonFile(true);
		//qDebug() << "编码区域变动,自动保存文件" << pythonFileName;
	}
	QFile pythonFile(pythonFileName);
	QByteArray temp;
	QByteArray data = python.toUtf8();
	if (pythonFile.open(QIODevice::ReadOnly)) {
		temp = pythonFile.readAll();
		pythonFile.close();
	}
	if (data != temp) {
		qInfo() << "编程区域变动，自动保存文件" << pythonFileName;
		//编程区域变动重置三维仿真记录数据
		m_pDeviceManage->reset3DStatus();
		if (pythonFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			pythonFile.write(python.toUtf8());
			pythonFile.close();
		}
	}
	//如果手动编写python内容为空则删除本地中的文件
	//因切换无人机设备是会清空上一次记录造成手动编写python传回空内容
	if (_WIDManual == id && true == python.isEmpty()) {
		qDebug() << "删除编写代码文件" << pythonFileName;
		pythonFile.remove();
	}
}

void UAVManage::onSocketDisconnected()
{
	qDebug() << "编程区域连接断开";
	m_pWebBlocklySocket = nullptr;
}

void UAVManage::onCurrentDeviceNameChanged(QString currentName, QString previousName)
{
	if (currentName.isEmpty()) return;
	qInfo() << "设备切换" << currentName << previousName;
	QString blocklyFilePath = getCurrentBlocklyFile();
	if (blocklyFilePath.isEmpty()) {
		onWebClear();
		return;
	}
	QFile file(blocklyFilePath);
	QByteArray arrBlockly;
	if (file.open(QIODevice::ReadOnly)) {
		arrBlockly = file.readAll();
		file.close();
	}
	QByteArray arrPythonCode;
	QFile filePython(getCurrentPythonFile(true));
	if (filePython.open(QIODevice::ReadOnly)) {
		arrPythonCode = filePython.readAll();
		filePython.close();
	}
	QJsonObject jsonObj;
	jsonObj.insert(_WMID, _WIDUpdate);
	jsonObj.insert("xml", arrBlockly.data());
	jsonObj.insert("name", currentName);
	if (false == arrPythonCode.isEmpty())jsonObj.insert("pythonCode", arrPythonCode.data());
	QJsonDocument jsonDoc(jsonObj);
	QByteArray data = jsonDoc.toJson();
	//qDebug() << "更新编程区域" << jsonDoc;
	if(m_pWebBlocklySocket) m_pWebBlocklySocket->sendTextMessage(data);
	
	//更新项目工程中选中设备
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string filename = code->fromUnicode(m_qstrCurrentProjectFile).data();
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root) return;
	tinyxml2::XMLElement* place = root->FirstChildElement(_ElementPlace_);
	if (!place) return;
	place->SetAttribute(_AttributeName_, currentName.toUtf8().data());
	error = doc.SaveFile(filename.c_str());
}

void UAVManage::onDeviceAdd(QString name, QString ip, float x, float y)
{
	if (m_qstrCurrentProjectFile.isEmpty()) return;
	qDebug() << "设备新增" << name << ip << x << y;
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string filename = code->fromUnicode(m_qstrCurrentProjectFile).data();
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root) return;
	//查找是否存在，已经存在则更新属性，否则新建
	tinyxml2::XMLElement* temp = root->FirstChildElement(_ElementDevice_);
	bool bExist = false;
	while (temp) {
		//遍历无人机属性
		QString devicename = temp->Attribute(_AttributeName_);
		if (devicename == name) {
			bExist = true;
			qDebug() << "新增设备已存在直接更新属性" << name;
			break;
		}
		temp = temp->NextSiblingElement(_ElementDevice_);
	}
	tinyxml2::XMLElement* device = temp;
	if (false == bExist) {
		device = doc.NewElement(_ElementDevice_);
		root->InsertEndChild(device);
		//新建设备文件
		QFileInfo info(m_qstrCurrentProjectFile);
		QString qstrPath = info.path();
		QString qstrBlocklyFile = QString("%1%2%3.blockly").arg(qstrPath).arg(_ProjectDirName_).arg(name);
		QString qstrPythonFile = QString("%1%2%3.py").arg(qstrPath).arg(_ProjectDirName_).arg(name);
		QFile file(qstrBlocklyFile);
		if (file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
			//当设备不存在写入默认blockly控件,默认ID为空，防止不同设备ID重复
			QString qstrBlock = "\
<xml xmlns=\"https://developers.google.com/blockly/xml\">\
  <block type=\"Fly_Takeoff\" id=\"\" x=\"88\" y=\"38\">\
    <field name=\"height\">100</field>\
    <next>\
      <block type=\"Fly_TimeGroup\" id=\"\">\
        <field name=\"minute\">0</field>\
        <field name=\"second\">5</field>\
        <next>\
          <block type=\"Fly_Land\" id=\"\"></block>\
        </next>\
      </block>\
    </next>\
  </block>\
</xml>\
";
			file.write(qstrBlock.toUtf8());
			file.waitForBytesWritten(1000);
			file.close();
		}
		file.setFileName(qstrPythonFile);
		if (file.open(QIODevice::ReadWrite | QIODevice::Truncate)) file.close();
	}
	device->SetAttribute(_AttributeName_, name.toUtf8().data());
#ifdef _UseUWBData_
	device->SetAttribute(_AttributeTag_, ip.toUtf8().data());
#else
	device->SetAttribute(_AttributeIP_, ip.toUtf8().data());
#endif
	device->SetAttribute(_AttributeX_, x);
	device->SetAttribute(_AttributeY_, y);
	error = doc.SaveFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
}

void UAVManage::onDeviceRemove(QString name)
{
	//删除无人机项目中文件
	if (m_qstrCurrentProjectFile.isEmpty()) return;
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string filename = code->fromUnicode(m_qstrCurrentProjectFile).data();
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root) return;
	tinyxml2::XMLElement* device = root->FirstChildElement(_ElementDevice_);
	if (!device) return;
	while (device) {
		//遍历无人机属性
		QString devicename = device->Attribute(_AttributeName_);
		if (devicename != name) {
			device = device->NextSiblingElement(_ElementDevice_);
			continue;
		}
		root->DeleteChild(device);
		break;
	}
	doc.SaveFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
	QFileInfo info(m_qstrCurrentProjectFile);
	QString qstrPath = info.path();
	QString qstrBlocklyFile = QString("%1%2%3.blockly").arg(qstrPath).arg(_ProjectDirName_).arg(name);
	QString qstrPythonFile = QString("%1%2%3.py").arg(qstrPath).arg(_ProjectDirName_).arg(name);
	QString qstrManualFile = QString("%1%2%3.mp").arg(qstrPath).arg(_ProjectDirName_).arg(name);
	QFile::remove(qstrBlocklyFile);
	QFile::remove(qstrPythonFile);
	QFile::remove(qstrManualFile);
}

void UAVManage::onDeviceRename(QString newName, QString oldName)
{
	if (m_qstrCurrentProjectFile.isEmpty()) return;
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string filename = code->fromUnicode(m_qstrCurrentProjectFile).data();
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root) return;
	tinyxml2::XMLElement* device = root->FirstChildElement(_ElementDevice_);
	if (!device) return;
	while (device) {
		//遍历无人机属性
		QString devicename = device->Attribute(_AttributeName_);
		if (devicename != oldName) {
			device = device->NextSiblingElement(_ElementDevice_);
			continue;
		}
		device->SetAttribute(_AttributeName_, newName.toUtf8().data());
		break;
	}
	doc.SaveFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
	QFileInfo info(m_qstrCurrentProjectFile);
	QString qstrPath = info.path();
	QString qstrNewBlocklyFile = QString("%1%2%3.blockly").arg(qstrPath).arg(_ProjectDirName_).arg(newName);
	QString qstrNewPythonFile = QString("%1%2%3.py").arg(qstrPath).arg(_ProjectDirName_).arg(newName);
	QString qstrNewManualFile = QString("%1%2%3.mp").arg(qstrPath).arg(_ProjectDirName_).arg(newName);
	QString qstrOldBlocklyFile = QString("%1%2%3.blockly").arg(qstrPath).arg(_ProjectDirName_).arg(oldName);
	QString qstrOldPythonFile = QString("%1%2%3.py").arg(qstrPath).arg(_ProjectDirName_).arg(oldName);
	QString qstrOldManualFile = QString("%1%2%3.mp").arg(qstrPath).arg(_ProjectDirName_).arg(oldName);
	QFile::rename(qstrOldBlocklyFile, qstrNewBlocklyFile);
	QFile::rename(qstrOldPythonFile, qstrNewPythonFile);
	QFile::rename(qstrOldManualFile, qstrNewManualFile);
}

void UAVManage::onDeviceResetIp(QString name, QString ip)
{
	if (m_qstrCurrentProjectFile.isEmpty()) return;
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string filename = code->fromUnicode(m_qstrCurrentProjectFile).data();
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root) return;
	tinyxml2::XMLElement* device = root->FirstChildElement(_ElementDevice_);
	if (!device) return;
	while (device) {
		//遍历无人机属性
		QString devicename = device->Attribute(_AttributeName_);
		if (devicename != name) {
			device = device->NextSiblingElement(_ElementDevice_);
			continue;
		}
#ifdef _UseUWBData_
		device->SetAttribute(_AttributeTag_, ip.toUtf8().data());
#else
		device->SetAttribute(_AttributeIP_, ip.toUtf8().data());
#endif
		break;
	}
	doc.SaveFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
}

void UAVManage::onDeviceResetLocation(QString name, long x, long y)
{
	if (m_qstrCurrentProjectFile.isEmpty()) return;
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string filename = code->fromUnicode(m_qstrCurrentProjectFile).data();
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root) return;
	tinyxml2::XMLElement* device = root->FirstChildElement(_ElementDevice_);
	if (!device) return;
	while (device) {
		//遍历无人机属性
		QString devicename = device->Attribute(_AttributeName_);
		if (devicename != name) {
			device = device->NextSiblingElement(_ElementDevice_);
			continue;
		}
		device->SetAttribute(_AttributeX_, x);
		device->SetAttribute(_AttributeY_, y);
		break;
	}
	doc.SaveFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
}

void UAVManage::onDeviceTakeoffFinished(bool takeoff)
{
	if (takeoff) {
		m_pMusicPlayer->startPlayMusic();
		//TODO 定时查询飞机状态，如果都飞完则关闭此窗口，同时关闭音乐
		m_pStopDialog->exec();
	}
	else {
		m_pStopDialog->close();
		m_pMusicPlayer->stopPlayMusic();
	}
}

void UAVManage::onAppMessage(const QString& message)
{
	
}

void UAVManage::onWaypointFinished(QString name, bool success, QString text)
{
	m_pDeviceManage->setEnabled(true);
	if (text.isEmpty()) return;
	if (success) _ShowInfoMessage(name + ": " + text);
	else _ShowErrorMessage(name + ": " + text);
}

void UAVManage::onUpdateMusic(QString qstrFilePath)
{
	QFileInfo info(qstrFilePath);
	if (!info.exists()) return;
	QString fileName = info.fileName();
	QFileInfo infoProject(m_qstrCurrentProjectFile);
	
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string filename = code->fromUnicode(m_qstrCurrentProjectFile).data();
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root) return;
	tinyxml2::XMLElement* place = root->FirstChildElement(_ElementPlace_);
	if (!place) return;
	//删除旧的音乐文件，先删除旧文件再复制新文件到工程目录，防止因文件名重复无法复制
	QString qstrOldMusic = infoProject.path() + _ProjectDirName_ + place->Attribute(_ElementMusic_);
	if (qstrOldMusic == qstrFilePath) {
		QMessageBox::warning(this, tr("错误"), tr("音乐文件相同，无法添加"));
		return;
	}
	QFile::remove(qstrOldMusic);
	//复制音乐文件到工程目录
	QString qstrNewFile = infoProject.path() + _ProjectDirName_ + fileName;
	if (qstrFilePath == qstrNewFile) return;
	if (false == QFile::copy(qstrFilePath, qstrNewFile)) {
		QMessageBox::warning(this, tr("提示"), tr("音乐文件添加失败"));
		return;
	}
	place->SetAttribute(_ElementMusic_, fileName.toUtf8().data());
	error = doc.SaveFile(filename.c_str());
	//音乐文件复制完成后添加音乐
	m_pMusicPlayer->updateLoadMusic(qstrNewFile);
}

void UAVManage::onCurrentMusicTime(unsigned int second)
{
	m_pDeviceManage->updateMusicTime(second);
}

void UAVManage::onCurrentPlayeState(qint8 state)
{
	//播放状态 1:开始 2 : 暂停 3 : 结束
	if (3 == state) m_pStopDialog->close();
}

void UAVManage::on3DDialogStauts(bool connect)
{
	if (connect) {
		qInfo() << "三维仿真窗口连接成功，准备发送航点数据到三维";
		if (m_pBackgrounMask) m_pBackgrounMask->visibleWidget();
		onMusicWaveFinished();
		m_pDeviceManage->waypointComposeAndUpload(m_qstrCurrentProjectFile, false);
	}
	else {
		if (m_pBackgrounMask) m_pBackgrounMask->close();
	}
}

void UAVManage::onMusicWaveFinished()
{
	m_pDeviceManage->setCurrentMusicPath(m_pMusicPlayer->getCurrentMusic(), m_pMusicPlayer->getMusicPixmap());
}

void UAVManage::onProjectAttribute()
{
	SpaceParam space(false, this);
	QFileInfo info(m_qstrCurrentProjectFile);
	space.setProjectPath(info.path());
	QPoint point = m_pDeviceManage->getSpaceSize();
	space.setSpaceSize(point.x(), point.y());
	if (QDialog::Accepted != space.exec()) return;

#ifdef _EditSpace_
	int xmax = space.getSpaceX();
	int ymax = space.getSpaceY();
	m_pDeviceManage->setSpaceSize(xmax, ymax);
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string filename = code->fromUnicode(m_qstrCurrentProjectFile).data();
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root) return;
	tinyxml2::XMLElement* place = root->FirstChildElement(_ElementPlace_);
	if (!place) return;
	place->SetAttribute(_AttributeX_, xmax);
	place->SetAttribute(_AttributeY_, ymax);
	error = doc.SaveFile(filename.c_str());
#endif
}

void UAVManage::onPrepareWidget()
{
	//展开或收缩控制列表界面
	if (m_pTopWidget->isHidden()) {
		m_pTopWidget->setGeometry(0, 0, width(), height());
		m_pTopWidget->show();
		m_pTopWidget->addWidget(m_pDeviceManage);
	}
	else {
		m_pTopWidget->close();
		m_pTopWidget->removeWidget(m_pDeviceManage);
		ui.layoutManger->addWidget(m_pDeviceManage);
	}
}

void UAVManage::onStart3DDialog()
{
	//没有添加音乐不能启动三维窗口
	QString qstrFilePath = m_pMusicPlayer->getCurrentMusic();
	if (qstrFilePath.isEmpty()) {
		QMessageBox::warning(this, tr("警告"), tr("未添加音乐文件无法使用三维仿真窗口"));
		return;
	}
	QFileInfo info(qstrFilePath);
	if (false == info.isFile()) {
		QMessageBox::warning(this, tr("警告"), tr("未添加音乐文件无法使用三维仿真窗口"));
		return;
	}
	if (false == QFile::exists(qstrFilePath)) {
		QMessageBox::warning(this, tr("警告"), tr("音乐文件无法使用无法使用三维仿真窗口"));
		return;
	}
	QFileInfo infoUe4Cache(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
	QString qstrUe4Path = infoUe4Cache.path() + "/UAV_Program_UE4/";
	deleteDir(qstrUe4Path);
	//UE4依赖文件判断
	if (false == QFile::exists("C:/WINDOWS/system32/x3daudio1_7.dll")
		|| false == QFile::exists("C:/WINDOWS/system32/D3DCOMPILER_43.dll")
		|| false == QFile::exists("C:/WINDOWS/system32/OPENGL32.dll")) {
		QProcess::startDetached(QApplication::applicationDirPath() + "/3D/Engine/Extras/Redist/en-us/UE4PrereqSetup_x64.exe");
		_ShowErrorMessage("三维仿真必要文件缺失，请安装后重试");
		return;
	}
	////先检查舞步编写是否正确
	//QString qstrErrorNames = m_pDeviceManage->waypointComposeAndUpload(m_qstrCurrentProjectFile, false);
	//if (false == qstrErrorNames.isEmpty()) {
	//	QString qstrErrorMsg = tr("检查舞步有错误，请修正后重试") + qstrErrorNames;
	//	_ShowErrorMessage(qstrErrorMsg);
	//	QMessageBox::warning(this, tr("警告"), qstrErrorMsg);
	//	return;
	//}

	if (m_pBackgrounMask == nullptr) {
		m_pBackgrounMask = new WaitingWidget(this);
	}
	m_pBackgrounMask->setGeometry(this->rect());
	m_pBackgrounMask->show();
	QDir::setCurrent(QApplication::applicationDirPath() + "/3D/UAV_Program_UE4/Binaries/Win64");
	m_p3DProcess->start("UAV_Program_UE4-Win64-Shipping.exe");
	m_p3DProcess->waitForStarted(1000);
	QDir::setCurrent(QApplication::applicationDirPath());
}

void UAVManage::onBlockFlicker(QString id)
{
	if (!m_pWebBlocklySocket) return;
	if (id.isEmpty()) return;
	QJsonObject jsonObj;
	jsonObj.insert(_WMID, _WIDBlockFlicker);
	jsonObj.insert("id", id);
	QJsonDocument doc(jsonObj);
	m_pWebBlocklySocket->sendTextMessage(doc.toJson());
}

void UAVManage::onTimerLogFile()
{
	//检查Log文件夹是否存在，删除存在时间长的文件
	qDebug() << "处理日志文件目录";
	QString qstrPath = QApplication::applicationDirPath() + "/Log";
	if (!QFile::exists(qstrPath)) return;
	QDir dir(qstrPath);
	QFileInfoList fileInfoList = dir.entryInfoList();
	QDateTime current = QDateTime::currentDateTime();
	foreach(QFileInfo info, fileInfoList) {
		if (info.fileName() == "." || info.fileName() == "..") continue;
		if (info.isDir()) {
			QString temp = info.filePath();
			qDebug() << "删除日志目录中多余的文件夹" << temp;
			deleteDir(temp);
		}
		else {
			//判断文件创建时间
			QDateTime time = info.lastModified();
			int days = time.daysTo(current);
			if (days > 7) {
				qDebug() << "删除旧日志文件" << info.filePath();
				QFile::remove(info.filePath());
			}
		}
	}
}

bool UAVManage::newProjectFile(QString qstrFile, unsigned int X, unsigned int Y)
{	
	const char* declaration = _XMLVersion_;
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.Parse(declaration);
	if (error != tinyxml2::XMLError::XML_SUCCESS) return false;
	tinyxml2::XMLElement* root = doc.NewElement(_ElementRoot_);
	doc.InsertEndChild(root);
	tinyxml2::XMLElement* user = doc.NewElement(_ElementPlace_);
	user->SetAttribute(_AttributeX_, X);
	user->SetAttribute(_AttributeY_, Y);
	QString version = QString("%1.%2.%3").arg(_MajorNumber_).arg(_MinorNumber_).arg(_BuildNumber_);
	user->SetAttribute(_QZVersion_, version.toUtf8().data());
	root->InsertEndChild(user);
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string name = code->fromUnicode(qstrFile).data();
	error = doc.SaveFile(name.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) {
		return false;
	}
	//新建项目后默认新建一个无人机设备
	m_qstrCurrentProjectFile = qstrFile;
	m_pDeviceManage->setCrrentProject(m_qstrCurrentProjectFile);
	onDeviceAdd(QString(_DeviceNamePrefix_) + QString::number(1), "", 100, 100);
	return true;
}

QToolButton* UAVManage::initMenuButton(QWidget* parent, QString text, QString noramlicon, QString activeicon, QMenu* menu)
{
	QToolButton* pButton = new QToolButton(this);
	pButton->setText(text);
	QIcon icon;
	icon.addFile(noramlicon, QSize(32, 32), QIcon::Normal, QIcon::Off);
	icon.addFile(activeicon, QSize(32, 32), QIcon::Active, QIcon::Off);
	pButton->setIcon(icon);
	pButton->setMenu(menu);
	pButton->setMinimumHeight(parent->height());
	pButton->setPopupMode(QToolButton::InstantPopup);
	pButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	pButton->setStyleSheet("QToolButton::menu-indicator{image:none;} \
							QToolButton:pressed{background:transparent;}");
	return pButton;
}
