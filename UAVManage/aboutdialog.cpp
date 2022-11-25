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
	ui.radioButton->setChecked(ParamReadWrite::readParam(_Update_).toBool());
	connect(ui.btnClose, &QAbstractButton::clicked, [this]() {  accept(); });
	connect(ui.btnRetry, &QAbstractButton::clicked, [this]() {  onCheckNewVersion(); });
	connect(ui.btnCheckVersion, &QAbstractButton::clicked, [this]() {  onCheckNewVersion(); });
	connect(ui.btnUpdate, &QAbstractButton::clicked, [this]() {  onStartUpdate(); });
	connect(ui.radioButton, &QAbstractButton::clicked, [this]() {
		ParamReadWrite::writeParam(_Update_, ui.radioButton->isChecked());
		});
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
	ui.stackedWidget->setCurrentIndex(1);
	QString qstrConnfigPath = QApplication::applicationDirPath() + _NewVersionPath_;
	//QString qstrUrl = "http://dldir1.qq.com/qqfile/qq/PCQQ9.6.9/QQ9.6.9.28878.exe";
	QString qstrUrl = QString("%1/%2").arg(_ServerUrl_).arg(_ConFile_);
	DownloadTool* download = new DownloadTool(qstrUrl, qstrConnfigPath);
	download->startDownload();
	connect(download, &DownloadTool::sigDownloadFinished, [this](QString error) {
		if (error.isEmpty()) {
			//解析升级配置文件，读取最新版本号
			QString config = QString("%1%2/%3").arg(QApplication::applicationDirPath()).arg(_NewVersionPath_).arg(_ConFile_);
			QString qstrNewVersionNumber = ParamReadWrite::readParam("version", AppVersion(), _Root_, config).toString();
			qDebug() << "新版本号" << qstrNewVersionNumber;
			QStringList list = qstrNewVersionNumber.split(".");
			if (list.count() != 3) {
				QMessageBox::warning(this, tr("错误"), tr("获取新版本号失败"));
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
				QMessageBox::warning(this, tr("错误"), tr("更新文件错误"));
				return;
			}
		}
		else {
			ui.stackedWidget->setCurrentIndex(0);
		}
		});
}

void AboutDialog::onStartUpdate()
{
	ui.stackedWidget->setCurrentIndex(2);
	QString qstrConnfigPath = QApplication::applicationDirPath() + _NewVersionPath_;
	QString qstrUrl = QString("%1/%2").arg(_ServerUrl_).arg(m_qstrNewVersionName);
	DownloadTool* download = new DownloadTool(qstrUrl, qstrConnfigPath);
	download->startDownload();
	connect(download, &DownloadTool::sigProgress, [this](qint64 bytesRead, qint64 totalBytes, qreal progress) {
		//文件下载进度
		ui.progressBar->setValue(progress * 100);
		ui.labelProgress->setText(QString(" %1%").arg(QString::number(progress * 100, 'f', 1)));
		});
	connect(download, &DownloadTool::sigDownloadFinished, [this](QString error) {
		if (error.isEmpty()) {
			QMessageBox::question(this, tr("询问"), tr("新版本下载完成，是否重启更新？"));
			return;
		}
		else {
			ui.stackedWidget->setCurrentIndex(1);
			QMessageBox::warning(this, tr("错误"), tr("更新文件下载失败，请重试"));
			return;
		}
		});
}

void AboutDialog::showEvent(QShowEvent* event)
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
	m_pLabelBackground = new QLabel(dynamic_cast<QWidget*>(parent()));
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);"));
	m_pLabelBackground->setFixedSize(dynamic_cast<QWidget*>(parent())->size());
	m_pLabelBackground->show();
	onCheckNewVersion();
}

void AboutDialog::hideEvent(QHideEvent* event)
{
	if (m_pLabelBackground) m_pLabelBackground->close();
}
