#include "aboutdialog.h"
#include <QGraphicsDropShadowEffect>
#include "definesetting.h"
#include "paramreadwrite.h"
#include "downloadtool.h"
#include <QDebug>
#include <QTimer>
#include <QProcess>
#include <QMessageBox>

AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	m_pLabelBackground = nullptr;
	m_pLabelProgress = nullptr;
	m_bShowing = false;
	m_bAutoUpdate = false;
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Dialog);
	this->setAttribute(Qt::WA_TranslucentBackground);
	ui.labelVersion->setText("V " + AppVersion());
	ui.stackedWidget->setCurrentIndex(1);
	ui.pageCheck->setVisible(false);
	ui.btnRestart->setVisible(false);
	m_bAutoUpdate = ParamReadWrite::readParam(_Update_, true).toBool();
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
	if (m_bAutoUpdate) {
		//延时检查更新，防止阻塞程序
		QTimer::singleShot(3000, this, SLOT(onCheckNewVersion()));
	}
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
	QString qstrUrl = QString("%1/%2").arg(_ServerUrl_).arg(_VersionFile_);
	DownloadTool* download = new DownloadTool(qstrUrl, qstrSavePath);
	download->startDownload();
	connect(download, &DownloadTool::sigDownloadFinished, [this](QString error) {
		if (error.isEmpty()) {
			//解析升级配置文件，读取最新版本号
			QString config = QString("%1%2/%3").arg(QApplication::applicationDirPath()).arg(_NewVersionPath_).arg(_VersionFile_);
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
			bool bUpdate = false;
			if (list.at(0).toUInt() > _MajorNumber_) {
				bUpdate = true;
			}
			else if (list.at(0).toUInt() >= _MajorNumber_ && list.at(1).toUInt() > _MinorNumber_) {
				bUpdate = true;
			}
			else if (list.at(0).toUInt() >= _MajorNumber_ && list.at(1).toUInt() >= _MinorNumber_ && list.at(2).toUInt() > _BuildNumber_) {
				bUpdate = true;
			}
			else {
				return;
			}
			if (false == bUpdate) return;
			ui.btnUpdate->setVisible(true);
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
	//安装文件下载到临时目录，防止出现下载文件不完整
	QString qstrSavePath = QApplication::applicationDirPath() + "/temp";
	QString qstrUrl = QString("%1/%2").arg(_ServerUrl_).arg(m_qstrNewVersionName);
	//qstrUrl = "http://downmini.yun.kugou.com/web/kugou_10112.exe";
	//qstrUrl = "http://dl-cdn.oray.com/sunlogin/windows/SunloginClient_13.1.0.48900_x64.exe";
	DownloadTool* download = new DownloadTool(qstrUrl, qstrSavePath);
	download->startDownload();
	//TODO 需要增加下载超时
	connect(download, &DownloadTool::sigProgress, [this](qint64 bytesRead, qint64 totalBytes, qreal progress) {
		//文件下载进度
		if (nullptr == m_pLabelProgress) {
			m_pLabelProgress = new QLabel(ui.pageProgress);
			m_pLabelProgress->setFixedSize(22, 22);
			m_pLabelProgress->setScaledContents(true);
			m_pLabelProgress->setPixmap(QPixmap(":/res/images/update_progress.png"));
		}
		if (m_pLabelProgress) {
			m_pLabelProgress->show();
			int x = ui.progressBar->geometry().x() + ui.progressBar->width() * progress - m_pLabelProgress->width() / 2;
			int y = ui.progressBar->geometry().y() - m_pLabelProgress->height();
			m_pLabelProgress->move(x, y);
		}
		ui.progressBar->setValue(progress * 1000);
		ui.labelProgress->setText(QString(" %1%").arg(QString::number(progress * 100, 'f', 1)));
		});
	connect(download, &DownloadTool::sigDownloadFinished, [this](QString error) {
		if (error.isEmpty()) {
			qInfo() << "更新文件下载完成" << m_qstrNewVersionName;
			QString qstrNewFile = QString("%1%2/%3").arg(QApplication::applicationDirPath()).arg(_NewVersionPath_).arg(m_qstrNewVersionName);
			bool bCopy = QFile::rename(QApplication::applicationDirPath() + "/temp/" + m_qstrNewVersionName, qstrNewFile);
			if (false == bCopy) {
				qWarning() << "更新文件移动失败" << m_qstrNewVersionName;
				ui.stackedWidget->setCurrentIndex(1);
				if (m_bShowing) QMessageBox::warning(this, tr("错误"), tr("更新文件下载失败，请重试"));
				return;
			}
			ui.btnRestart->setVisible(true);
			if(nullptr == m_pLabelBackground) backgroundShow();
			QMessageBox::StandardButton button = QMessageBox::question(this, tr("询问"), tr("新版本下载完成，是否重启更新？"));
			if(false == m_bShowing) m_pLabelBackground->close();
			if(QMessageBox::Yes == button) onRestartApp();
			return;
		}
		else {
			ui.stackedWidget->setCurrentIndex(1);
			if (m_bShowing) {
				qWarning() << "更新文件下载失败" << m_qstrNewVersionName;
				QMessageBox::warning(this, tr("错误"), tr("更新文件下载失败，请重试"));
			}
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
	qInfo() << "重启程序";
	qApp->quit();
	QProcess::startDetached(qApp->applicationFilePath());
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
