#include "aboutdialog.h"
#include <QGraphicsDropShadowEffect>
#include "definesetting.h"
#include "paramreadwrite.h"
#include "downloadtool.h"
#include <QDebug>
#include <QMessageBox>

#define _ServerUrl_ "E:/fly/UAVManage/x64/Release/"
#define _ConFile_   "version.ini"
AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_pLabelBackground = nullptr;
	m_bShowing = false;
	m_bAutoUpdate = false;
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Tool);
	this->setAttribute(Qt::WA_TranslucentBackground);
	QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
	shadow->setOffset(0, 0);
	shadow->setColor(QColor("#444444"));
	shadow->setBlurRadius(10);
	this->setGraphicsEffect(shadow);
	ui.labelVersion->setText("V " + AppVersion());
	ui.stackedWidget->setCurrentIndex(1);
	ui.pageCheck->setVisible(false);
	ui.btnRestart->setVisible(false);
	m_bAutoUpdate = ParamReadWrite::readParam(_Update_).toBool();
	ui.radioButton->setChecked(m_bAutoUpdate);
	connect(ui.btnClose, &QAbstractButton::clicked, [this]() {  accept(); });
	connect(ui.btnRetry, &QAbstractButton::clicked, [this]() {  onCheckNewVersion(); });
	connect(ui.btnCheckVersion, &QAbstractButton::clicked, [this]() {  onCheckNewVersion(); });
	connect(ui.btnUpdate, &QAbstractButton::clicked, [this]() {  onStartUpdate(); });
	connect(ui.btnRestart, &QAbstractButton::clicked, [this]() {  onRestartApp(); });
	connect(ui.radioButton, &QAbstractButton::clicked, [this]() {
		ParamReadWrite::writeParam(_Update_, ui.radioButton->isChecked());
		});
	//自动后台更新
	if (m_bAutoUpdate) onCheckNewVersion();
}

AboutDialog::~AboutDialog()
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
}

void AboutDialog::onCheckNewVersion()
{
	if (ui.stackedWidget->currentIndex() == 2) {
		qDebug() << "正在更新中";
		return;
	}
	ui.stackedWidget->setCurrentIndex(1);
	QString qstrSavePath = QApplication::applicationDirPath() + _NewVersionPath_;
	QString qstrUrl = QString("%1/%2").arg(_ServerUrl_).arg(_ConFile_);
	DownloadTool* download = new DownloadTool(qstrUrl, qstrSavePath);
	download->startDownload();
	connect(download, &DownloadTool::sigDownloadFinished, [this](QString error) {
		if (error.isEmpty()) {
			//解析升级配置文件，读取最新版本号
			QString config = QString("%1%2/%3").arg(QApplication::applicationDirPath()).arg(_NewVersionPath_).arg(_ConFile_);
			QString qstrNewVersionNumber = ParamReadWrite::readParam("version", AppVersion(), _Root_, config).toString();
			qDebug() << "新版本号" << qstrNewVersionNumber;
			QStringList list = qstrNewVersionNumber.split(".");
			if (list.count() != 3) {
				if(m_bShowing) QMessageBox::warning(this, tr("错误"), tr("获取新版本号失败"));
				return;
			}
			ui.pageCheck->setVisible(true);
			ui.btnUpdate->setVisible(false);
			ui.labelNewVersionNumber->setText("V "+qstrNewVersionNumber);
			if (list.at(0).toUInt() > _MajorNumber_ || list.at(1).toUInt() > _MinorNumber_ || list.at(2).toUInt() > _BuildNumber_) {
				//有新版本
				ui.btnUpdate->setVisible(true);
			}
			m_qstrNewVersionName = ParamReadWrite::readParam("file", AppVersion(), _Root_, config).toString();
			if (m_qstrNewVersionName.isEmpty()) {
				if (m_bShowing) QMessageBox::warning(this, tr("错误"), tr("更新文件错误"));
				return;
			}
			if (m_bAutoUpdate) {
				onStartUpdate();
			}
		}
		else {
			ui.stackedWidget->setCurrentIndex(0);
		}
		});
}

void AboutDialog::onStartUpdate()
{
	ui.btnRestart->setVisible(false);
	ui.stackedWidget->setCurrentIndex(2);
	QString qstrSavePath = QApplication::applicationDirPath() + _NewVersionPath_;
	QString qstrUrl = QString("%1/%2").arg(_ServerUrl_).arg(m_qstrNewVersionName);
	qstrUrl = "http://downmini.yun.kugou.com/web/kugou_10112.exe";
	DownloadTool* download = new DownloadTool(qstrUrl, qstrSavePath);
	download->startDownload();
	//TODO 需要增加下载超时
	connect(download, &DownloadTool::sigProgress, [this](qint64 bytesRead, qint64 totalBytes, qreal progress) {
		//文件下载进度
		ui.progressBar->setValue(progress * 100);
		ui.labelProgress->setText(QString(" %1%").arg(QString::number(progress * 100, 'f', 1)));
		});
	connect(download, &DownloadTool::sigDownloadFinished, [this](QString error) {
		if (error.isEmpty()) {
			ui.btnRestart->setVisible(true);
			if(nullptr == m_pLabelBackground) backgroundShow();
			QMessageBox::StandardButton button = QMessageBox::question(this, tr("询问"), tr("新版本下载完成，是否重启更新？"));
			if(false == m_bShowing) m_pLabelBackground->close();
			if(QMessageBox::Yes == button) onRestartApp();
			return;
		}
		else {
			ui.stackedWidget->setCurrentIndex(1);
			if (m_bShowing) QMessageBox::warning(this, tr("错误"), tr("更新文件下载失败，请重试"));
			return;
		}
		});
}

void AboutDialog::backgroundShow()
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
	m_pLabelBackground = new QLabel(dynamic_cast<QWidget*>(parent()));
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);"));
	m_pLabelBackground->setFixedSize(dynamic_cast<QWidget*>(parent())->size());
	m_pLabelBackground->show();
}

void AboutDialog::onRestartApp()
{
	qDebug() << "重启程序";
}

void AboutDialog::showEvent(QShowEvent* event)
{
	m_bShowing = true;
	m_bAutoUpdate = false;
	backgroundShow();
	onCheckNewVersion();
}

void AboutDialog::hideEvent(QHideEvent* event)
{
	m_bShowing = false;
	if (m_pLabelBackground) m_pLabelBackground->close();
}
