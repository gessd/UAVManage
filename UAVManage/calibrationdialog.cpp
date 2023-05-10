#include "calibrationdialog.h"
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include "messagelistdialog.h"

#define _GifPro "gif"
CalibrationDialog::CalibrationDialog(int calib, DeviceControl* device, QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	//setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Dialog);
	//窗口关闭自动释放
	setAttribute(Qt::WA_DeleteOnClose, true);
	m_pLabelBackground = nullptr;
	m_pMovieAcc = nullptr;
	m_bAccFinished = false;
	ui.stackedWidgetProgress->setVisible(false);
	connect(device, &DeviceControl::sigLogMessage, this, &CalibrationDialog::onDeviceMessage);
	connect(device, &DeviceControl::sigConnectStatus, this, &CalibrationDialog::onDeviceStatus);
	ui.stackedWidgetProgress->setCurrentIndex(calib);
	if (_Accelerometer == calib || _Magnetometer == calib) {
		//加计或磁罗盘校准校准
		ui.stackedWidgetProgress->setCurrentIndex(_Accelerometer);
		if (_Accelerometer == calib) {
			ui.labelAccText->setText("请按上方图示放置无人机，然后保持静止直到接收到校准数据并点亮下图中的小图标");
			ui.labelAcc_1->setProperty(_GifPro, ":/res/acc/behind.gif");
			ui.labelAcc_2->setProperty(_GifPro, ":/res/acc/front.gif");
			ui.labelAcc_3->setProperty(_GifPro, ":/res/acc/left.gif");
			ui.labelAcc_4->setProperty(_GifPro, ":/res/acc/right.gif");
			ui.labelAcc_5->setProperty(_GifPro, ":/res/acc/bottom.gif");
			ui.labelAcc_6->setProperty(_GifPro, ":/res/acc/top.gif");
			ui.labelAccGif->setFixedSize(300, 300);
		}
		else if (_Magnetometer == calib){
			ui.labelAccText->setText("请按图示旋转无人机，直到接收到校准数据并点亮下图中的小图标");
			ui.labelAcc_1->setProperty(_GifPro, ":/res/magnet/z+.gif");
			ui.labelAcc_2->setProperty(_GifPro, ":/res/magnet/z-.gif");
			ui.labelAcc_3->setProperty(_GifPro, ":/res/magnet/x-.gif");
			ui.labelAcc_4->setProperty(_GifPro, ":/res/magnet/x+.gif");
			ui.labelAcc_5->setProperty(_GifPro, ":/res/magnet/y-.gif");
			ui.labelAcc_6->setProperty(_GifPro, ":/res/magnet/y+.gif");
			ui.labelAcc_1->setPixmap(QPixmap(":/res/magnet/z+.gif"));
			ui.labelAcc_2->setPixmap(QPixmap(":/res/magnet/z-.gif"));
			ui.labelAcc_3->setPixmap(QPixmap(":/res/magnet/x-.gif"));
			ui.labelAcc_4->setPixmap(QPixmap(":/res/magnet/x+.gif"));
			ui.labelAcc_5->setPixmap(QPixmap(":/res/magnet/y-.gif"));
			ui.labelAcc_6->setPixmap(QPixmap(":/res/magnet/y+.gif"));
			ui.labelAccGif->setFixedSize(400, 300);
		}
		ui.stackedWidgetProgress->setVisible(true);
		m_pMovieAcc = new QMovie(ui.labelAcc_1->property(_GifPro).toString());
		ui.labelAccGif->setMovie(m_pMovieAcc);
		m_pMovieAcc->start();
		connect(m_pMovieAcc, &QMovie::finished, [this]() { 
			//重复播放动图
			if(false == m_bAccFinished) QTimer::singleShot(3000, [this]() { if(m_pMovieAcc) m_pMovieAcc->start(); });
			});
	}
}

CalibrationDialog::~CalibrationDialog()
{
	if (m_pMovieAcc) {
		m_pMovieAcc->stop();
		m_pMovieAcc->deleteLater();
		m_pMovieAcc = nullptr;
	}
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
}

void CalibrationDialog::addLogToBrowser(QString text)
{
	qDebug() << "校准过程:" << text;
	ui.textBrowser->append(QDateTime::currentDateTime().toString("[hh:mm:ss.zzz]:")+ text);
}

void CalibrationDialog::onDeviceMessage(QString data)
{
	if (data.isEmpty()) return;
	addLogToBrowser(data);
	if(m_pMovieAcc) m_pMovieAcc->stop();
	//加计校准的6个面 
	if (data.contains("Acc -x data get")) {
		ui.labelAcc_1->setEnabled(true);
	}
	else if (data.contains("Acc +x data get")) {
		ui.labelAcc_2->setEnabled(true);
	}
	else if (data.contains("Acc -y data get")) {
		ui.labelAcc_3->setEnabled(true);
	}
	else if (data.contains("Acc +y data get")) {
		ui.labelAcc_4->setEnabled(true);		
	}
	else if (data.contains("Acc -z data get")) {
		ui.labelAcc_5->setEnabled(true);
	}
	else if (data.contains("Acc +z data get")) {
		ui.labelAcc_6->setEnabled(true);
	}
	else if (data.contains(_AccFinished_)) {
		//加计校准完成
		m_pMovieAcc->stop();
		m_bAccFinished = true;
		_ShowInfoMessage(tr("加计校准完成"));
		close();
	}
	//磁罗盘校准的6个面
	else if (data.contains("Mag +z data get")) {
		ui.labelAcc_1->setEnabled(true);
	}
	else if (data.contains("Mag -z data get")) {
		ui.labelAcc_2->setEnabled(true);
	}
	else if (data.contains("Mag -x data get")) {
		ui.labelAcc_3->setEnabled(true);
	}
	else if (data.contains("Mag +x data get")) {
		ui.labelAcc_4->setEnabled(true);
	}
	else if (data.contains("Mag -y data get")) {
		ui.labelAcc_5->setEnabled(true);
	}
	else if (data.contains("Mag +y data get")) {
		ui.labelAcc_6->setEnabled(true);
	}
	else if (data.contains("mag calb success")) {
		//磁罗盘校准完成
		m_pMovieAcc->stop();
		m_bAccFinished = true;
		_ShowInfoMessage(tr("磁罗盘校准完成"));
		close();
	}
	if (false == m_bAccFinished && m_pMovieAcc) {
		QObjectList list = ui.widgetAccProgress->children();
		foreach(QObject* pObj, list) {
			if (nullptr == pObj) continue;
			QLabel* pLabel = dynamic_cast<QLabel*>(pObj);
			if (nullptr == pLabel) continue;
			if (pLabel->isEnabled()) continue;
			QString qstrGif = pLabel->property(_GifPro).toString();
			m_pMovieAcc->setFileName(qstrGif);
			m_pMovieAcc->start();
			break;
		}
	}
}

void CalibrationDialog::onDeviceStatus(QString name, QString ip, bool connect)
{
	addLogToBrowser(connect ? "连接成功" : "<font color=red>连接断开</font>");
}

void CalibrationDialog::showEvent(QShowEvent* event)
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
	QWidget* pWidget = this;
	while (true) {
		QWidget* pTemp = dynamic_cast<QWidget*>(pWidget->parent());
		if (!pTemp) break;
		pWidget = pTemp;
	}
	m_pLabelBackground = new QLabel(pWidget);
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);"));
	m_pLabelBackground->setFixedSize(pWidget->size());
	m_pLabelBackground->show();
	if (m_pMovieAcc) m_pMovieAcc->start();
}

void CalibrationDialog::hideEvent(QHideEvent* event)
{
	if (m_pLabelBackground) m_pLabelBackground->close();
}
