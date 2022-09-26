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

UAVManage::UAVManage(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	m_pSocketServer = nullptr;
	m_pWebSocket = nullptr;
	m_pDeviceManage = nullptr;
	m_p3DProcess = nullptr;
	
	MessageListDialog::getInstance()->setParent(this);
	//程序初始化
	connect(ui.webEngineView, SIGNAL(loadProgress(int)), this, SLOT(onWebLoadProgress(int)));
	connect(ui.webEngineView, SIGNAL(loadFinished(bool)), this, SLOT(onWebLoadFinished(bool)));
	//初始化web
	m_pSocketServer = new QWebSocketServer(QStringLiteral("Socket Server"), QWebSocketServer::NonSecureMode, this);
	m_pSocketServer->listen(QHostAddress::LocalHost, _WebSocketPort_);
	connect(m_pSocketServer, SIGNAL(newConnection()), this, SLOT(onSocketNewConnection()));

	//增加设备列表
    m_pDeviceManage = new DeviceManage(this);
	m_pDeviceManage->setEnabled(false);
	m_pDeviceManage->setMaximumWidth(200);
    ui.gridLayoutMain->addWidget(m_pDeviceManage, 0,1);
	connect(m_pDeviceManage, SIGNAL(currentDeviceNameChanged(QString, QString)), this, SLOT(onCurrentDeviceNameChanged(QString, QString)));
	connect(m_pDeviceManage, SIGNAL(deviceAddFinished(QString, QString, float, float)), this, SLOT(onDeviceAdd(QString, QString, float, float)));
	connect(m_pDeviceManage, SIGNAL(deviceRemoveFinished(QString)), this, SLOT(onDeviceRemove(QString)));
	connect(m_pDeviceManage, SIGNAL(deviceRenameFinished(QString, QString)), this, SLOT(onDeviceRename(QString, QString)));
	connect(m_pDeviceManage, &DeviceManage::sigWaypointProcess, this, &UAVManage::onWaypointProcess);
	connect(m_pDeviceManage, &DeviceManage::sigTakeoffFinished, this, &UAVManage::onDeviceTakeoffFinished);
	connect(m_pDeviceManage, &DeviceManage::sig3DDialogStatus, this, &UAVManage::on3DDialogStauts);

	//添加菜单
	QStyle* style = QApplication::style();
	QMenu* pTestMenu = new QMenu(tr("测试"));
	ui.menuBar->addMenu(pTestMenu);
	QAction* pQssAction = new QAction(style->standardIcon(QStyle::SP_ComputerIcon), tr("更新样式"));
	QAction* pMessage = new QAction("消息窗口");
	pTestMenu->addAction(pQssAction);
	pTestMenu->addAction(pMessage);
	connect(pQssAction, &QAction::triggered, [this]() { updateStyle(); });
	connect(pMessage, &QAction::triggered, [this]() { 
		_ShowErrorMessage(tr("这是错误消息这是错误消息这是错误消息这是错误消息这是错误消息这是错误消息这是错误消息"));
		_ShowWarningMessage(tr("警告消息警告消息警告消息警告消息警告消息警告消息警告消息警告消息"));
		_ShowInfoMessage(tr("正常提示消息正常提示消息正常提示消息正常提示消息正常提示消息正常提示消息正常提示消息正常提示消息正常提示消息"));
		});
	//项目管理菜单
	QMenu* pProjectMenu = new QMenu(tr("项目"));
	ui.menuBar->addMenu(pProjectMenu);
	QAction* pActionNew = new QAction(style->standardIcon(QStyle::SP_FileDialogNewFolder), tr("新建"));
	QAction* pActionOpen = new QAction(style->standardIcon(QStyle::SP_DialogOpenButton), tr("打开"));
	QAction* pActionSaveas = new QAction(style->standardIcon(QStyle::SP_DirIcon), tr("另存为"));
	pProjectMenu->addAction(pActionNew);
	pProjectMenu->addAction(pActionOpen);
	pProjectMenu->addAction(pActionSaveas);
	connect(pActionNew, &QAction::triggered, [this]() { onNewProject(); });
	connect(pActionOpen, &QAction::triggered, [this]() {
		QString qstrFile = QFileDialog::getOpenFileName(this, tr("项目名"), "", "File(*.qz)");
		if (qstrFile.isEmpty()) return;
		onOpenProject(qstrFile);
		});
	connect(pActionSaveas, &QAction::triggered, [this]() {onSaveasProject(); });

	//三维预览进程
	m_p3DProcess = new QProcess(this);
	//起飞准备菜单
	QMenu* pMenuFlyPrepare = new QMenu(tr("起飞准备"));
	ui.menuBar->addMenu(pMenuFlyPrepare);
	QAction* pActionFly1 = new QAction("1.检查舞步");
	QAction* pActionFly2 = new QAction("2.三维仿真");
	QAction* pActionFly3 = new QAction("3.基站标定");
	QAction* pActionFly4 = new QAction("4.上传舞步");
	QAction* pActionFly5 = new QAction("5.定桩授时");
	QAction* pActionFly6 = new QAction("6.准备起飞");
	pMenuFlyPrepare->addAction(pActionFly1);
	pMenuFlyPrepare->addAction(pActionFly2);
	pMenuFlyPrepare->addAction(pActionFly3);
	pMenuFlyPrepare->addAction(pActionFly4);
	pMenuFlyPrepare->addAction(pActionFly5);
	pMenuFlyPrepare->addAction(pActionFly6);
	connect(pActionFly1, &QAction::triggered, [this]() { deviceWaypoint(); });
	connect(pActionFly2, &QAction::triggered, [this]() { 
		//if (m_qstrCurrentProjectFile.isEmpty()) return;
		m_p3DProcess->start("notepad.exe");
		//m_p3DProcess->waitForFinished();
		});
	connect(pActionFly3, &QAction::triggered, [this]() { 
		if (m_qstrCurrentProjectFile.isEmpty()) return;
		PlaceInfoDialog info(this);
		info.exec();
		});
	connect(pActionFly4, &QAction::triggered, [this]() { deviceWaypoint(true); });
	connect(pActionFly5, &QAction::triggered, [this]() {  //暂无功能
		});
	connect(pActionFly6, &QAction::triggered, [this]() { 
		if (m_qstrCurrentProjectFile.isEmpty()) return;
		m_pDeviceManage->allDeviceControl(DeviceManage::_DeviceSetout); 
		});

	m_pSoundWidget = new SoundGrade(this);
	m_pSoundWidget->setEnabled(false);
	ui.horizontalLayoutSound->addWidget(m_pSoundWidget);
	connect(m_pSoundWidget, &SoundGrade::sigUpdateMusic, this, &UAVManage::onUpdateMusic);
	connect(m_pSoundWidget, &SoundGrade::sigMsuicTime, this, &UAVManage::onCurrentMusicTime);
	connect(m_pSoundWidget, &SoundGrade::playeState, this, &UAVManage::onCurrentPlayeState);
}

UAVManage::~UAVManage()
{
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

QString UAVManage::getCurrentPythonFile()
{
	if (m_qstrCurrentProjectFile.isEmpty() || !m_pDeviceManage) return QString();
	QString name = m_pDeviceManage->getCurrentDeviceName();
	if (name.isEmpty()) return QString();
	QFileInfo info(m_qstrCurrentProjectFile);
	QString path = info.path();
	return QString("%1%2%3.py").arg(path).arg(_ProjectDirName_).arg(name);
}

void UAVManage::onNewProject()
{
	m_pDeviceManage->setEnabled(false);
	m_pSoundWidget->setEnabled(false);
	//先清空数据
	if (false == m_qstrCurrentProjectFile.isEmpty()) {
		QFileInfo info(m_qstrCurrentProjectFile);
		_ShowInfoMessage(tr("关闭工程")+ info.baseName());
	}
	m_qstrCurrentProjectFile.clear();
	onWebClear();
	if(m_pDeviceManage) m_pDeviceManage->clearDevice();
	//选择新建路径
	QString qstrName = QFileDialog::getSaveFileName(this, tr("项目名"), "", "File(*.qz)");
	if (qstrName.isEmpty()) return;
	QFileInfo info(qstrName);
	QString qstrDir = info.path() + "/" + info.baseName();
	QString qstrFile = qstrDir + "/" + info.fileName();
	//新建项目文件夹
	QDir dir;
	if (!dir.mkdir(qstrDir)) {
		_ShowErrorMessage(info.baseName() + tr("项目创建失败"));
		return;
	}
	//新建项目文件并写入初始化内容
	if (!newProjectFile(qstrFile)) {
		dir.rmdir(qstrDir);
		_ShowErrorMessage(info.baseName() + tr("创建项目文件失败"));
		return;
	}
	dir.mkdir(qstrDir + _ProjectDirName_);
	_ShowInfoMessage(info.baseName()+tr("新建项目完成"));
	onOpenProject(qstrFile);
}

void UAVManage::onOpenProject(QString qstrFile)
{
	qDebug() << "----打开工程" << qstrFile;
	m_qstrCurrentProjectFile.clear();
	onWebClear();
	if (m_pDeviceManage) m_pDeviceManage->clearDevice();
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
	float x = place->FloatAttribute(_AttributeX_);
	float y = place->FloatAttribute(_AttributeY_);
	QFileInfo infoProject(qstrFile);
	QString qstrMusicFilePath = infoProject.path() + _ProjectDirName_ + place->Attribute(_ElementMusic_);
	QString qstrCurrnetName = place->Attribute(_AttributeName_);
	tinyxml2::XMLElement* device = root->FirstChildElement(_ElementDevice_);
	if (!device) {
		_ShowErrorMessage(tr("工程中没有添加设备"));
		return;
	}
	m_qstrCurrentProjectFile = qstrFile;
	while (device){
		//遍历无人机属性
		QString devicename = device->Attribute(_AttributeName_);
		QString ip = device->Attribute(_AttributeIP_);
		float dx = device->FloatAttribute(_AttributeX_);
		float dy = device->FloatAttribute(_AttributeY_);
		device = device->NextSiblingElement(_ElementDevice_);
		if (m_pDeviceManage) {
			QString error = m_pDeviceManage->addDevice(devicename, ip, dx, dy);
			if (!error.isEmpty()) {
				_ShowErrorMessage(tr("无法添加设备") + devicename + error);
			}
		}
	}
	_ShowInfoMessage(tr("打开工程完成"));
	qDebug() << "----工程打开完成";
	m_pDeviceManage->setCurrentDevice(qstrCurrnetName);
	m_pSoundWidget->updateLoadMusic(qstrMusicFilePath);
	m_pDeviceManage->setCurrentMusicPath(qstrMusicFilePath);
	m_pDeviceManage->setEnabled(true);
	m_pSoundWidget->setEnabled(true);
	ParamReadWrite::writeParam(_Path_, m_qstrCurrentProjectFile);
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
	QString qstrName = QFileDialog::getSaveFileName(this, tr("项目名"), "", "File(*.qz)");
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

void UAVManage::onWebClear()
{
	if (!m_pWebSocket) return;
	m_pWebSocket->sendTextMessage("");
}

void UAVManage::showEvent(QShowEvent* event)
{
	if(!m_pWebSocket) loadWeb();
}

void UAVManage::closeEvent(QCloseEvent* event)
{
	MessageListDialog::getInstance()->exitDialog();
}

void UAVManage::resizeEvent(QResizeEvent* event)
{
	MessageListDialog::getInstance()->move((width() - MessageListDialog::getInstance()->width()) / 2, 0);
}

void UAVManage::onWebLoadProgress(int progress)
{
	//网页加载进度
}

void UAVManage::onWebLoadFinished(bool finished)
{
	//网页加载完成
	static bool bInit = false;
	if (bInit) return;
	bInit = true;
	QString qstrPath = ParamReadWrite::readParam(_Path_).toString();
	if (qstrPath.isEmpty()) return;
	if (!QFile::exists(qstrPath)) return;
	onOpenProject(qstrPath);
}

void UAVManage::onSocketNewConnection()
{
	//不能同时打开两个软件，否则端口占用无法使用
	//只记录一个web连接，防止通过浏览器访问blockly
	if (m_pWebSocket) return;
	m_pWebSocket = m_pSocketServer->nextPendingConnection();
	connect(m_pWebSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(onSocketTextMessageReceived(QString)));
	connect(m_pWebSocket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
}

void UAVManage::onSocketTextMessageReceived(QString message)
{
	//防止数据大量重复处理
	if (m_qstrLastWebMessage == message) return;
	m_qstrLastWebMessage = message;
	//更新blockly及python文件
	message = message.right(message.size() - 7);						//减去 JsonData 字符串
	QStringList strList = message.split("pythonData");
	if (strList.size() < 2) return;
	QString xmlData = strList[0];										//XML文本数据
	xmlData = xmlData.right(xmlData.length() - 1);
	QString pythonData = strList[1];									//python文本数据
	pythonData = pythonData.right(pythonData.length() - 1);
	qDebug() << xmlData;
	qDebug() << pythonData;
	QString blocklyFileName = getCurrentBlocklyFile();
	QString pythonFileName = getCurrentPythonFile();
	qDebug() << "---update file" << blocklyFileName << pythonFileName;
	if (blocklyFileName.isEmpty() || pythonFileName.isEmpty()) return;
	QFile blocklyFile(blocklyFileName);
	if (blocklyFile.open(QIODevice::WriteOnly | QIODevice::Text)){
		blocklyFile.write(xmlData.toUtf8());
		blocklyFile.close();
	}
	QFile pythonFile(pythonFileName);
	if (pythonFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		pythonFile.write(pythonData.toUtf8());
		pythonFile.close();
	}
}

void UAVManage::onSocketDisconnected()
{
	m_pWebSocket = nullptr;
}

void UAVManage::onCurrentDeviceNameChanged(QString currentName, QString previousName)
{
	qDebug() << "----设备切换" << currentName << previousName;
	QString blocklyFilePath = getCurrentBlocklyFile();
	if (blocklyFilePath.isEmpty()) return;
	QFile file(blocklyFilePath);
	QByteArray arrBlockly;
	if (file.open(QIODevice::ReadOnly)) {
		arrBlockly = file.readAll();
		file.close();
	}
	qDebug() << "----更新WEB界面" << blocklyFilePath << arrBlockly;
	if(m_pWebSocket) m_pWebSocket->sendTextMessage(arrBlockly);
	
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
	qDebug() << "----设备新增" << name << ip << x << y;
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
			qDebug() << "----新增设备已存在直接更新属性" << name;
			break;
		}
		temp = temp->NextSiblingElement(_ElementDevice_);
	}
	tinyxml2::XMLElement* device = temp;
	if (!bExist) {
		device = doc.NewElement(_ElementDevice_);
		root->InsertEndChild(device);
		//新建设备文件
		QFileInfo info(m_qstrCurrentProjectFile);
		QString qstrPath = info.path();
		QString qstrBlocklyFile = QString("%1%2%3.blockly").arg(qstrPath).arg(_ProjectDirName_).arg(name);
		QString qstrPythonFile = QString("%1%2%3.py").arg(qstrPath).arg(_ProjectDirName_).arg(name);
		QFile file(qstrBlocklyFile);
		if (file.open(QIODevice::ReadWrite)) file.close();
		file.setFileName(qstrPythonFile);
		if (file.open(QIODevice::ReadWrite)) file.close();
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
	QFile::remove(qstrBlocklyFile);
	QFile::remove(qstrPythonFile);
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
	QString qstrOldBlocklyFile = QString("%1%2%3.blockly").arg(qstrPath).arg(_ProjectDirName_).arg(oldName);
	QString qstrOldPythonFile = QString("%1%2%3.py").arg(qstrPath).arg(_ProjectDirName_).arg(oldName);
	QFile::rename(qstrOldBlocklyFile, qstrNewBlocklyFile);
	QFile::rename(qstrOldPythonFile, qstrNewPythonFile);
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
		_ShowInfoMessage(name + ": " + text + Utility::waypointMessgeFromStatus(res));
	} else{
		_ShowErrorMessage(name + ": " + text + Utility::waypointMessgeFromStatus(res));
	}
}

void UAVManage::onUpdateMusic(QString qstrFilePath)
{
	QFileInfo info(qstrFilePath);
	if (!info.exists()) return;
	QString fileName = info.fileName();
	QFileInfo infoProject(m_qstrCurrentProjectFile);
	QString qstrNewFile = infoProject.path() + _ProjectDirName_ + fileName;
	if (qstrFilePath == qstrNewFile) return;
	QFile::copy(qstrFilePath, qstrNewFile);
	m_pDeviceManage->setCurrentMusicPath(qstrNewFile);

	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string filename = code->fromUnicode(m_qstrCurrentProjectFile).data();
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root) return;
	tinyxml2::XMLElement* place = root->FirstChildElement(_ElementPlace_);
	if (!place) return;
	QString qstrOldMusic = infoProject.path() + _ProjectDirName_ + place->Attribute(_ElementMusic_);
	QFile::remove(qstrOldMusic);
	place->SetAttribute(_ElementMusic_, fileName.toUtf8().data());
	error = doc.SaveFile(filename.c_str());
}

void UAVManage::onCurrentMusicTime(int second)
{
	m_pDeviceManage->setUpdateWaypointTime(second);
}

void UAVManage::onCurrentPlayeState(qint8 state)
{
	//播放状态 1:开始 2 : 暂停 3 : 结束
	m_pDeviceManage->setCurrentPlayeState(state);
}

void UAVManage::on3DDialogStauts(bool connect)
{
	if (connect) {
		deviceWaypoint();
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
		QString qstrMusicPath = infoProject.path() + _ProjectDirName_ + place->Attribute(_ElementMusic_);
		m_pDeviceManage->setCurrentMusicPath(qstrMusicPath);
	}
}

bool UAVManage::newProjectFile(QString qstrFile, float X, float Y)
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
	tinyxml2::XMLElement* device = doc.NewElement(_ElementDevice_);
	QString qstrDeviceName = QString(_DeviceNamePrefix_) + QString::number(1);
	device->SetAttribute(_AttributeName_, qstrDeviceName.toUtf8().data());
	device->SetAttribute(_AttributeIP_, "");
	device->SetAttribute(_AttributeX_, 0);
	device->SetAttribute(_AttributeY_, 0);
	user->SetAttribute(_AttributeName_, qstrDeviceName.toUtf8().data());
	root->InsertEndChild(device);
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string name = code->fromUnicode(qstrFile).data();
	error = doc.SaveFile(name.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) {
		return false;
	}
	return true;
	
}

void UAVManage::deviceWaypoint(bool bUpload)
{
	if (m_qstrCurrentProjectFile.isEmpty()) return;
	QStringList list = m_pDeviceManage->getDeviceNameList();
	foreach(QString name, list) {
		if (name.isEmpty()) continue;
		QFileInfo infoProject(m_qstrCurrentProjectFile);
		QString qstrDevicePyFile = infoProject.path() + _ProjectDirName_ + name + _PyFileSuffix_;
		if (false == QFile::exists(qstrDevicePyFile)) continue;
		QFile file(qstrDevicePyFile);
		if (!file.open(QIODevice::ReadOnly)) continue;
		QByteArray arrData = file.readAll();
		file.close();
		if (arrData.isEmpty()) {
			_ShowErrorMessage(name + tr(": 没有编写舞步"));
			continue;
		}
		//生成舞步过程必须一个个生成，python交互函数是静态全局，所以同时只能执行一个设备生成舞步
		if (!ptyhon.compilePythonCode(arrData)) {
			//生成舞步失败
			_ShowErrorMessage(name + tr(": 解析舞步积木块失败"));
			continue;
		}
		while (!ptyhon.isFinished()){
			QApplication::processEvents();
		}
		if (PythonSuccessful != ptyhon.getLastState()) {
			_ShowErrorMessage(name + tr(": 舞步转换失败"));
			continue;
		}
		QVector<NavWayPointData> data = ptyhon.getWaypointData();
		if (0 == data.count()) {
			_ShowWarningMessage(name+tr("没有舞步信息"));
			continue;
		}
		else{
			if(!bUpload) _ShowInfoMessage(name + tr("生成舞步完成"));
		}

		//上传舞步到飞控/更新三维舞步
		QString message = m_pDeviceManage->sendWaypoint(name, data, bUpload);
		if(!message.isEmpty()) _ShowWarningMessage(name + tr("上传舞步") + message);
	}
}
