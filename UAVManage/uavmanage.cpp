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

void UAVManage::updateWeb()
{
	QString qstrUrl = "file:///" + QApplication::applicationDirPath() + "/blockly_dev/blockly_dev/code/index.html";
	ui.webEngineView->load(QUrl(qstrUrl));
}

void UAVManage::onNewProject()
{
	//先清空数据
	updateWeb();
	m_pDeviceManage->clearDevice();
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
		//dir.rmdir(qstrDir);
		QMessageBox::warning(this, tr("错误"), tr("新建项目文件失败"));
		return;
	}
	onOpenProject(qstrFile);
}

void UAVManage::onOpenProject(QString qstrFile)
{
	updateWeb();
	m_pDeviceManage->clearDevice();
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
	while (device){
		//遍历无人机属性
		QString devicename = device->Attribute(_AttributeName_);
		QString ip = device->Attribute(_AttributeIP_);
		float dx = device->FloatAttribute(_AttributeX_);
		float dy = device->FloatAttribute(_AttributeY_);
		qDebug() << x << y << devicename << ip << dx << dy;
		device = device->NextSiblingElement(_ElementDevice_);
		m_pDeviceManage->addDevice(devicename, ip, dx, dy);
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

void UAVManage::showEvent(QShowEvent* event)
{
	updateWeb();
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
	connect(m_pWebSocket, SIGNAL(textMessageReceived(const QString)), this, SLOT(onSocketTextMessageReceived(const QString)));
	connect(m_pWebSocket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
}

void UAVManage::onSocketTextMessageReceived(const QString message)
{
	qDebug() << "----websocket message:" << message;
}

void UAVManage::onSocketDisconnected()
{
	qDebug() << "----websocket Disconnected";
	m_pWebSocket = nullptr;
}

void UAVManage::onCurrentDeviceNameChanged(QString currentName, QString previousName)
{
	//updateWeb();
}

void UAVManage::onBtnTestClicked()
{
    
}

bool UAVManage::newProjectFile(QString qstrFile, float X, float Y)
{
	/*
	QTextCodec* code = QTextCodec::codecForName(_XMLNameCoding_);
	std::string name = code->fromUnicode(qstrFile).data();

	tinyxml2::XMLDocument* doc = new tinyxml2::XMLDocument;
	tinyxml2::XMLDeclaration* declaration = doc->NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\"");
	doc->LinkEndChild(declaration);

	tinyxml2::XMLElement* School = doc->NewElement("School");
	doc->LinkEndChild(School);
	School->SetAttribute("name", "机械工程学院");

	tinyxml2::XMLElement* Class = doc->NewElement("Class");
	School->LinkEndChild(Class);
	Class->SetAttribute("name", "c++");

	tinyxml2::XMLElement* Student = doc->NewElement("Student");
	Class->LinkEndChild(Student);
	Student->SetAttribute("name", "天霸");
	Student->SetAttribute("number", "01");

	tinyxml2::XMLElement* Email = doc->NewElement("Email");
	Student->LinkEndChild(Email);
	tinyxml2::XMLText* email = doc->NewText("TB@126.com");
	Email->LinkEndChild(email);

	tinyxml2::XMLElement* Address = doc->NewElement("Address");
	Student->LinkEndChild(Address);
	tinyxml2::XMLText* address = doc->NewText("中国辽宁");
	Address->LinkEndChild(address);

	tinyxml2::XMLElement* Student_1 = doc->NewElement("Student");
	Class->LinkEndChild(Student_1);
	Student_1->SetAttribute("name", "动霸");
	Student_1->SetAttribute("number", "02");

	tinyxml2::XMLElement* Email_1 = doc->NewElement("Email");
	Student_1->LinkEndChild(Email_1);
	tinyxml2::XMLText* email_1 = doc->NewText("DB@126.com");
	Email_1->LinkEndChild(email_1);

	tinyxml2::XMLElement* Address_1 = doc->NewElement("Address");
	Student_1->LinkEndChild(Address_1);
	tinyxml2::XMLText* address_1 = doc->NewText("中国香港");
	Address_1->LinkEndChild(address_1);
	if (tinyxml2::XMLError::XML_SUCCESS != doc->SaveFile(name.c_str())) {
		delete doc;
		return false;
	}
	delete doc;
	return true;
	*/
	
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
