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
#include "definesetting.h"
#include "tinyxml2/tinyxml2.h"
#include "messagelistdialog.h"
#include "placeinfodialog.h"
#include "SoundGrade.h"
#include "paramreadwrite.h"
#include "spaceparam.h"
#include "waitingwidget.h"
#include "qxtglobalshortcut.h"
#include "historymessage.h"
#include "registerdialog.h"

UAVManage::UAVManage(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	m_pSocketServer = nullptr;
	m_pWebBlocklySocket = nullptr;
	m_pDeviceManage = nullptr;
	m_p3DProcess = nullptr;
	m_pBackgrounMask = nullptr;
	//注册事件过滤器，处理快捷键事件
	installEventFilter(this);
	MessageListDialog::getInstance()->setParent(this);
	m_pHistory = new HistoryMessage(this);
	connect(MessageListDialog::getInstance(), SIGNAL(sigMessage(QString, _Messagelevel, bool)), m_pHistory, SLOT(onMessageData(QString, _Messagelevel, bool)));
	//程序初始化
	connect(ui.webEngineView, SIGNAL(loadProgress(int)), this, SLOT(onWebLoadProgress(int)));
	connect(ui.webEngineView, SIGNAL(loadFinished(bool)), this, SLOT(onWebLoadFinished(bool)));
	//初始化web
	m_pSocketServer = new QWebSocketServer(QStringLiteral("Socket Server"), QWebSocketServer::NonSecureMode, this);
	m_pSocketServer->listen(QHostAddress::Any, _WebSocketPort_);
	connect(m_pSocketServer, SIGNAL(newConnection()), this, SLOT(onSocketNewConnection()));

	//增加设备列表
	m_pDeviceManage = new DeviceManage(this);
	m_pDeviceManage->setEnabled(false);
	m_pDeviceManage->setMaximumWidth(200);
	ui.gridLayoutMain->addWidget(m_pDeviceManage, 0, 1);
	connect(m_pDeviceManage, &DeviceManage::currentDeviceNameChanged, this, &UAVManage::onCurrentDeviceNameChanged);
	connect(m_pDeviceManage, &DeviceManage::deviceAddFinished, this, &UAVManage::onDeviceAdd);
	connect(m_pDeviceManage, &DeviceManage::deviceRemoveFinished, this, &UAVManage::onDeviceRemove);
	connect(m_pDeviceManage, &DeviceManage::deviceRenameFinished, this, &UAVManage::onDeviceRename);
	connect(m_pDeviceManage, &DeviceManage::deviceResetIp, this, &UAVManage::onDeviceResetIp);
	connect(m_pDeviceManage, &DeviceManage::deviceResetLocation, this, &UAVManage::onDeviceResetLocation);
	connect(m_pDeviceManage, &DeviceManage::sigWaypointProcess, this, &UAVManage::onWaypointProcess);
	connect(m_pDeviceManage, &DeviceManage::sigTakeoffFinished, this, &UAVManage::onDeviceTakeoffFinished);
	connect(m_pDeviceManage, &DeviceManage::sig3DDialogStatus, this, &UAVManage::on3DDialogStauts);

	m_pAbout = new AboutDialog(this);
	m_pSoundWidget = new SoundGrade(this);
	m_pSoundWidget->setEnabled(false);
	ui.horizontalLayoutSound->addWidget(m_pSoundWidget);
	connect(m_pSoundWidget, &SoundGrade::sigUpdateMusic, this, &UAVManage::onUpdateMusic);
	connect(m_pSoundWidget, &SoundGrade::sigMsuicTime, this, &UAVManage::onCurrentMusicTime);
	connect(m_pSoundWidget, &SoundGrade::playeState, this, &UAVManage::onCurrentPlayeState);
	connect(m_pSoundWidget, &SoundGrade::updateMusicWaveFinished, this, &UAVManage::onMusicWaveFinished);
	initMenu();
}

UAVManage::~UAVManage()
{
	if (m_p3DProcess) {
		m_p3DProcess->close();
		m_p3DProcess->kill();
		m_p3DProcess->deleteLater();
		m_p3DProcess = nullptr;
	}
	if (m_pSoundWidget) {
		delete m_pSoundWidget;
		m_pSoundWidget = nullptr;
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
	//添加菜单
	ui.menuBar->setMinimumHeight(30);
	QWidget* pMenuWidget = new QWidget(ui.menuBar);
	QHBoxLayout* pMenuLayout = new QHBoxLayout(pMenuWidget);
	pMenuLayout->setSpacing(10);
	pMenuLayout->setMargin(0);
	pMenuWidget->setLayout(pMenuLayout);
	pMenuWidget->setGeometry(0, 0, ui.menuBar->width(), ui.menuBar->height());

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
	QAction* pActionFly1 = new QAction(QIcon(":/res/images/inspect.png"), tr("检查舞步"));
	QAction* pActionFly2 = new QAction(QIcon(":/res/images/stereoscopic.png"), tr("三维仿真"));
	QAction* pActionFly3 = new QAction(QIcon(":/res/images/basestation.png"), tr("基站标定"));
	QAction* pActionFly4 = new QAction(QIcon(":/res/images/upload.png"), tr("上传舞步"));
	QAction* pActionFly5 = new QAction(QIcon(":/res/images/time.png"), tr("定桩授时"));
	QAction* pActionFly6 = new QAction(QIcon(":/res/images/prepare.png"), tr("准备起飞"));
	pMenuFlyPrepare->addAction(pActionFly1);
	pMenuFlyPrepare->addAction(pActionFly2);
	pMenuFlyPrepare->addAction(pActionFly3);
	pMenuFlyPrepare->addAction(pActionFly4);
	pMenuFlyPrepare->addAction(pActionFly5);
	pMenuFlyPrepare->addAction(pActionFly6);
	ui.toolBar->layout()->setSpacing(10);
	ui.toolBar->addAction(pActionFly1);
	ui.toolBar->addAction(pActionFly2);
	ui.toolBar->addAction(pActionFly3);
	ui.toolBar->addAction(pActionFly4);
	ui.toolBar->addAction(pActionFly5);
	ui.toolBar->addAction(pActionFly6);

	pMenuLayout->addWidget(initMenuButton(pMenuWidget, tr(""), ":/res/logo/qz_logo.ico", ":/res/logo/qz_logo.ico", pIconMenu));
	pMenuLayout->addWidget(initMenuButton(pMenuWidget, tr("项目"), ":/res/menu/P01_file_open_btn_cli.png", ":/res/menu/P01_file_open_btn_cli.png", pProjectMenu));
	m_pButtonFlyPrepare = initMenuButton(pMenuWidget, tr("起飞准备"), ":/res/menu/preparation.png", ":/res/menu/preparation.png", pMenuFlyPrepare);
	m_pButtonFlyPrepare->setEnabled(false);
	ui.toolBar->setEnabled(false);
	pMenuLayout->addWidget(m_pButtonFlyPrepare);
	QMenu* pActionHelp = new QMenu(tr("帮助"));
	pActionHelp->setWindowFlags(pMenuFlyPrepare->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
	pActionHelp->setAttribute(Qt::WA_TranslucentBackground);
	QAction* pActionAbout = new QAction(QIcon(":/res/menu/P02_help_about_btn_new_ic.png"), tr("版本"));
	pActionHelp->addAction(pActionAbout);
	pMenuLayout->addWidget(initMenuButton(pMenuWidget, tr("帮助"), ":/res/menu/P02_help_about_page_ic.png", ":/res/menu/P02_help_about_page_ic.png", pActionHelp));
	connect(pActionAbout, &QAction::triggered, [this]() {m_pAbout->exec(); });
	m_pRegister = new RegisterDialog(this);
	QAction* pActionReg = new QAction(QIcon(":/res/logo/qz_logo.ico"), tr("授权"));
	connect(pActionReg, &QAction::triggered, [this]() {m_pRegister->exec(); });
	pActionHelp->addAction(pActionReg);

	connect(pActionFly1, &QAction::triggered, [this]() { m_pDeviceManage->waypointComposeAndUpload(m_qstrCurrentProjectFile, false); });
	connect(pActionFly2, &QAction::triggered, [this]() {
		//没有添加音乐不能启动三维窗口
		QString qstrFilePath = m_pSoundWidget->getCurrentMusic();
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
		//UE4依赖文件判断
		if (false == QFile::exists("C:/Windows/System32/D3DX9_43.dll") && false == QFile::exists("C:/Windows/SysWOW64/D3DX9_43.dll")) {
			QProcess::startDetached(QApplication::applicationDirPath() + "/3D/Engine/Extras/Redist/en-us/UE4PrereqSetup_x64.exe");
			_ShowErrorMessage("三维仿真必要文件缺失，请安装后重试");
			return;
		}
		if (m_pBackgrounMask == nullptr) {
			m_pBackgrounMask = new WaitingWidget(this);
		}
		m_pBackgrounMask->setGeometry(this->rect());
		m_pBackgrounMask->show();
		QDir::setCurrent(QApplication::applicationDirPath() + "/3D/UAV_Program_UE4/Binaries/Win64");
		m_p3DProcess->start("UAV_Program_UE4-Win64-Shipping.exe");
		m_p3DProcess->waitForStarted(1000);
		QDir::setCurrent(QApplication::applicationDirPath());
		});
	connect(pActionFly3, &QAction::triggered, [this]() {
		if (m_qstrCurrentProjectFile.isEmpty()) return;
		PlaceInfoDialog info(m_pDeviceManage->getSpaceSize(), this);
		info.exec();
		//不允许更新场地大小
		return;
		if (false == info.isValidStation()) return;
		QMap<QString, QPoint> map = info.getStationAddress();
		m_pDeviceManage->setStationAddress(map);
		//根据基站判断场地大小，修改场地范围
		QStringList keys = map.keys();
		int xmax = 0;
		int ymax = 0;
		foreach(QString name, keys) {
			xmax = qMax(xmax, map.value(name).x());
			ymax = qMax(ymax, map.value(name).y());
		}
		m_pDeviceManage->setSpaceSize(xmax, ymax);
		if (m_qstrCurrentProjectFile.isEmpty()) return;
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
		});
	connect(pActionFly4, &QAction::triggered, [this]() { m_pDeviceManage->waypointComposeAndUpload(m_qstrCurrentProjectFile, true); });
	connect(pActionFly5, &QAction::triggered, [this]() {  //暂无功能
		});
	connect(pActionFly6, &QAction::triggered, [this]() {
		if (m_qstrCurrentProjectFile.isEmpty()) return;
		m_pDeviceManage->allDeviceControl(_DeviceSetout);
		});
	pMenuLayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
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
	QString qstrFile = qstrDir + "/" + info.fileName() + ".qz";
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
		_ShowErrorMessage(tr("打开工程文件失败"));
		return;
	}
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root) {
		_ShowErrorMessage(tr("读取工程文件失败"));
		return;
	}
	tinyxml2::XMLElement* place = root->FirstChildElement(_ElementPlace_);
	if (!place) {
		_ShowErrorMessage(tr("工程文件损坏,无法读取"));
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
		jsonObj.insert("z", 10000);
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
	while (device){
		//遍历无人机属性
		QString devicename = device->Attribute(_AttributeName_);
		QString ip = device->Attribute(_AttributeIP_);
		int x = device->IntAttribute(_AttributeX_);
		int y = device->IntAttribute(_AttributeY_);
		device = device->NextSiblingElement(_ElementDevice_);
		if (m_pDeviceManage) {
			QString error = m_pDeviceManage->addDevice(devicename, ip, x, y);
			if (!error.isEmpty()) {
				_ShowErrorMessage(tr("无法使用设备") + devicename + error);
			}
		}
	}
	_ShowInfoMessage(tr("打开工程完成"));
	QFileInfo info(m_qstrCurrentProjectFile);
	QString name = info.baseName();
	m_pDeviceManage->setCurrentDevice(qstrCurrnetName);
	m_pSoundWidget->updateLoadMusic(qstrMusicFilePath);
	m_pDeviceManage->setEnabled(true);
	m_pSoundWidget->setEnabled(true);
	m_pButtonFlyPrepare->setEnabled(true);
	ui.toolBar->setEnabled(true);
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
}

void UAVManage::onCloseProject()
{
	//清空项目信息
	m_pSoundWidget->stopPlayMusic();
	m_pSoundWidget->clearSound();
	if (m_pDeviceManage) m_pDeviceManage->clearDevice();
	onWebClear();
	m_pActionAttribute->setEnabled(false);
	m_pDeviceManage->setEnabled(false);
	m_pSoundWidget->setEnabled(false);
	m_pButtonFlyPrepare->setEnabled(false);
	ui.toolBar->setEnabled(false);
	//先清空数据
	if (false == m_qstrCurrentProjectFile.isEmpty()) {
		QFileInfo info(m_qstrCurrentProjectFile);
		_ShowInfoMessage(tr("关闭工程") + info.baseName());
	}
	m_qstrCurrentProjectFile.clear();
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
		initGlobalShortcut("Ctrl+Space");
		loadWeb();
	}
	m_pHistory->resetWidget();	
}

void UAVManage::closeEvent(QCloseEvent* event)
{
	MessageListDialog::getInstance()->exitDialog();
}

void UAVManage::resizeEvent(QResizeEvent* event)
{
	MessageListDialog::getInstance()->move((width() - MessageListDialog::getInstance()->width()) / 2, 0);
	m_pHistory->resetWidget();
}

bool UAVManage::eventFilter(QObject* watched, QEvent* event)
{
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
	return false;
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
	qDebug() << "积木块变动,自动保存编程文件";
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
	qDebug() << "更新文件" << blocklyFileName << pythonFileName;
	if (blocklyFileName.isEmpty() || pythonFileName.isEmpty()) return;
	if (_WIDUpdate == id) {
		QFile blocklyFile(blocklyFileName);
		if (blocklyFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
			blocklyFile.write(xml.toUtf8());
			blocklyFile.close();
		}
	}
	if (_WIDManual == id) {
		pythonFileName = getCurrentPythonFile(true);
	}
	QFile pythonFile(pythonFileName);
	if (pythonFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		pythonFile.write(python.toUtf8());
		pythonFile.close();
	}
	//如果手动编写python内容为空则删除本地中的文件
	//因切换无人机设备是会清空上一次记录造成手动编写python传回空内容
	if (_WIDManual == id && true == python.isEmpty()) {
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
	qDebug() << "设备切换" << currentName << previousName;
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
	qDebug() << "更新编程区域";
	QJsonObject jsonObj;
	jsonObj.insert(_WMID, _WIDUpdate);
	jsonObj.insert("xml", arrBlockly.data());
	jsonObj.insert("name", currentName);
	if (false == arrPythonCode.isEmpty())jsonObj.insert("pythonCode", arrPythonCode.data());
	QJsonDocument jsonDoc(jsonObj);
	QByteArray data = jsonDoc.toJson();
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
			//当设备不存在写入默认blockly控件
			QString qstrBlock = "\
<xml xmlns=\"https://developers.google.com/blockly/xml\">\
  <block type=\"FlyTakeoff\" id=\"RHfWI7IcqrRM~UK}OEnp\" x=\"88\" y=\"38\">\
    <field name=\"height\">100</field>\
    <next>\
      <block type=\"FlyTimeGroup\" id=\"W5WBWFi9LI[Xy{I/4OG3\">\
        <field name=\"minute\">0</field>\
        <field name=\"second\">1</field>\
        <next>\
          <block type=\"FlyLand\" id=\"4X4;~Xwrm@Qa53W;3T2.\"></block>\
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
	device->SetAttribute(_AttributeIP_, ip.toUtf8().data());
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
		device->SetAttribute(_AttributeIP_, ip.toUtf8().data());
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
	if(takeoff) m_pSoundWidget->startPlayMusic();
	else m_pSoundWidget->stopPlayMusic();
}

void UAVManage::onAppMessage(const QString& message)
{
	
}

void UAVManage::onWaypointProcess(QString name, unsigned int index, unsigned int count, int res, bool finish, QString text)
{
	if (finish) {
		m_pDeviceManage->setEnabled(true);
	}
	if (text.isEmpty()) return;
	if (_DeviceStatus::DeviceDataSucceed == res) {
		//舞步上传完成并成功
		_ShowInfoMessage(name + ": " + text + Utility::waypointMessgeFromStatus(_DeviceWaypoint, res));
	} else{
		_ShowErrorMessage(name + ": " + text + Utility::waypointMessgeFromStatus(_DeviceWaypoint, res));
	}
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
	QFile::remove(qstrOldMusic);
	//复制音乐文件到工程目录
	QString qstrNewFile = infoProject.path() + _ProjectDirName_ + fileName;
	if (qstrFilePath == qstrNewFile) return;
	if (false == QFile::copy(qstrFilePath, qstrNewFile)) {
		QMessageBox::warning(this, tr("提示"), tr("音乐使用失败"));
		return;
	}
	place->SetAttribute(_ElementMusic_, fileName.toUtf8().data());
	error = doc.SaveFile(filename.c_str());
	//音乐文件复制完成后添加音乐
	m_pSoundWidget->updateLoadMusic(qstrNewFile);
}

void UAVManage::onCurrentMusicTime(unsigned int second)
{
	m_pDeviceManage->updateMusicTime(second);
}

void UAVManage::onCurrentPlayeState(qint8 state)
{
	//播放状态 1:开始 2 : 暂停 3 : 结束
	m_pDeviceManage->setCurrentPlayeState(state);
}

void UAVManage::on3DDialogStauts(bool connect)
{
	if (connect) {
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
	m_pDeviceManage->setCurrentMusicPath(m_pSoundWidget->getCurrentMusic(), m_pSoundWidget->getMusicPixmap());
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
	root->InsertEndChild(user);
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string name = code->fromUnicode(qstrFile).data();
	error = doc.SaveFile(name.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) {
		return false;
	}
	//新建项目后默认新建一个无人机设备
	m_qstrCurrentProjectFile = qstrFile;
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
