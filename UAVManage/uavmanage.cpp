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

UAVManage::UAVManage(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	m_pSocketServer = nullptr;
	m_pWebSocket = nullptr;
	m_pDeviceManage = nullptr;

	//程序初始化
	connect(ui.webEngineView, SIGNAL(loadProgress(int)), this, SLOT(onWebLoadProgress(int)));
	connect(ui.webEngineView, SIGNAL(loadFinished(bool)), this, SLOT(onWebLoadFinished(bool)));
	//初始化web
	m_pSocketServer = new QWebSocketServer(QStringLiteral("Socket Server"), QWebSocketServer::NonSecureMode, this);
	m_pSocketServer->listen(QHostAddress::LocalHost, _WebSocketPort_);
	connect(m_pSocketServer, SIGNAL(newConnection()), this, SLOT(onSocketNewConnection()));

	//增加设备列表
    m_pDeviceManage = new DeviceManage(this);
	m_pDeviceManage->setMaximumWidth(200);
    ui.gridLayoutMain->addWidget(m_pDeviceManage, 0,1);
	connect(m_pDeviceManage, SIGNAL(currentDeviceNameChanged(QString, QString)), this, SLOT(onCurrentDeviceNameChanged(QString, QString)));
	connect(m_pDeviceManage, SIGNAL(deviceAddFinished(QString, QString, float, float)), this, SLOT(onDeviceAdd(QString, QString, float, float)));
	connect(m_pDeviceManage, SIGNAL(deviceRemoveFinished(QString)), this, SLOT(onDeviceRemove(QString)));
	connect(m_pDeviceManage, SIGNAL(deviceRenameFinished(QString, QString)), this, SLOT(onDeviceRename(QString, QString)));

	//添加菜单
	QStyle* style = QApplication::style();
	QMenu* pProjectMenu = new QMenu(tr("项目"));
	QAction* pActionNew = new QAction(style->standardIcon(QStyle::SP_FileDialogNewFolder), tr("新建"));
	QAction* pActionOpen = new QAction(style->standardIcon(QStyle::SP_DialogOpenButton), tr("打开"));
	QAction* pActionSave = new QAction(style->standardIcon(QStyle::SP_DialogSaveButton), tr("保存"));
	QAction* pActionSaveas = new QAction(style->standardIcon(QStyle::SP_DirIcon), tr("另存为"));
	pProjectMenu->addAction(pActionNew);
	pProjectMenu->addAction(pActionOpen);
	pProjectMenu->addAction(pActionSave);
	pProjectMenu->addAction(pActionSaveas);
	ui.menuBar->addMenu(pProjectMenu);
	connect(pActionNew, &QAction::triggered, [this]() { onNewProject(); });
	connect(pActionOpen, &QAction::triggered, [this]() {
		QString qstrFile = QFileDialog::getOpenFileName(this, tr("项目名"), "", "File(*.qz)");
		onOpenProject(qstrFile);
		});
	connect(pActionSave, &QAction::triggered, [this]() {onSaveProject(); });
	connect(pActionSaveas, &QAction::triggered, [this]() {onSaveasProject(); });
}

UAVManage::~UAVManage()
{
	if (m_pSocketServer) {
		m_pSocketServer->close();
		delete m_pSocketServer;
		m_pSocketServer = nullptr;
	}
	if (m_pDeviceManage) {
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
	//先清空数据
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
		QMessageBox::warning(this, tr("错误"), tr("新建项目失败"));
		return;
	}
	//新建项目文件并写入初始化内容
	if (!newProjectFile(qstrFile)) {
		dir.rmdir(qstrDir);
		QMessageBox::warning(this, tr("错误"), tr("新建项目文件失败"));
		return;
	}
	dir.mkdir(qstrDir + _ProjectDirName_);
	onOpenProject(qstrFile);
}

void UAVManage::onOpenProject(QString qstrFile)
{
	m_qstrCurrentProjectFile.clear();
	onWebClear();
	if (m_pDeviceManage) m_pDeviceManage->clearDevice();
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string filename = code->fromUnicode(qstrFile).data();
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root) return;
	tinyxml2::XMLElement* place = root->FirstChildElement(_ElementPlace_);
	if (!place) return;
	//读取场地大小
	float x = place->FloatAttribute(_AttributeX_);
	float y = place->FloatAttribute(_AttributeY_);
	tinyxml2::XMLElement* device = root->FirstChildElement(_ElementDevice_);
	if (!device) return;
	m_qstrCurrentProjectFile = qstrFile;
	while (device){
		//遍历无人机属性
		QString devicename = device->Attribute(_AttributeName_);
		QString ip = device->Attribute(_AttributeIP_);
		float dx = device->FloatAttribute(_AttributeX_);
		float dy = device->FloatAttribute(_AttributeY_);
		qDebug() << x << y << devicename << ip << dx << dy;
		device = device->NextSiblingElement(_ElementDevice_);
		if (m_pDeviceManage) m_pDeviceManage->addDevice(devicename, ip, dx, dy);
	}
}

void UAVManage::onSaveProject()
{
	//暂时使用自动保存
}

void UAVManage::onSaveasProject()
{
	//复制工程到新文件夹并重命名
}

void UAVManage::onWebClear()
{
	if (!m_pWebSocket) return;
	m_pWebSocket->sendTextMessage("");
}

void UAVManage::showEvent(QShowEvent* event)
{
	loadWeb();
}

void UAVManage::onWebLoadProgress(int progress)
{
	qDebug() << "----webload:" << progress;
}

void UAVManage::onWebLoadFinished(bool finished)
{
	qDebug() << "----webFinished:" << finished;
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
	qDebug() << "----websocket message:" << message;
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
	qDebug() << "----websocket Disconnected";
	m_pWebSocket = nullptr;
}

void UAVManage::onCurrentDeviceNameChanged(QString currentName, QString previousName)
{
	QString blocklyFilePath = getCurrentBlocklyFile();
	if (blocklyFilePath.isEmpty()) return;
	QFile file(blocklyFilePath);
	if (!file.open(QIODevice::ReadOnly)) return;
	QByteArray arrBlockly = file.readAll();
	if(m_pWebSocket) m_pWebSocket->sendTextMessage(arrBlockly);
	file.close();
}

void UAVManage::onDeviceAdd(QString name, QString ip, float x, float y)
{
	if (m_qstrCurrentProjectFile.isEmpty()) return;
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string filename = code->fromUnicode(m_qstrCurrentProjectFile).data();
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError error = doc.LoadFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
	tinyxml2::XMLElement* root = doc.RootElement();
	if (!root) return;
	tinyxml2::XMLElement* device = doc.NewElement(_ElementDevice_);
	device->SetAttribute(_AttributeName_, name.toUtf8().data());
	device->SetAttribute(_AttributeIP_, ip.toUtf8().data());
	device->SetAttribute(_AttributeX_, x);
	device->SetAttribute(_AttributeY_, y);
	root->InsertEndChild(device);
	error = doc.SaveFile(filename.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) return;
	onWebClear();
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
	device->SetAttribute(_AttributeName_, (QString(_DeviceNamePrefix_)+QString::number(1)).toUtf8().data());
	device->SetAttribute(_AttributeIP_, "");
	device->SetAttribute(_AttributeX_, 0);
	device->SetAttribute(_AttributeY_, 0);
	root->InsertEndChild(device);
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string name = code->fromUnicode(qstrFile).data();
	error = doc.SaveFile(name.c_str());
	if (error != tinyxml2::XMLError::XML_SUCCESS) {
		return false;
	}
	return true;
	
}
