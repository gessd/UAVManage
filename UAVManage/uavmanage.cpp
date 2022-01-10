#include "uavmanage.h"
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include "definesetting.h"

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
	connect(pActionOpen, &QAction::triggered, [this]() {onOpenProject(); });
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
	updateWeb();
	m_pDeviceManage->clearDevice();
	m_pDeviceManage->addDevice("", "");
}

void UAVManage::onOpenProject()
{

}

void UAVManage::onSaveProject()
{

}

void UAVManage::onSaveasProject()
{

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
	if (m_pWebSocket) return;
	m_pWebSocket = m_pSocketServer->nextPendingConnection();
	qDebug() << "----websocket NewConnection" << m_pWebSocket;
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
	updateWeb();
}

void UAVManage::onBtnTestClicked()
{
    
}
