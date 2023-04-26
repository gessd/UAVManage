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
	m_pLabelBackground = nullptr;
	m_pMovieAcc = nullptr;
	m_bAccFinished = false;
	ui.btnAccFinished->setVisible(false);
	ui.widgetAccProgress->setVisible(false);
	ui.labelAcc_1->setProperty(_GifPro, ":/res/acc/top.gif");
	ui.labelAcc_2->setProperty(_GifPro, ":/res/acc/bottom.gif");
	ui.labelAcc_3->setProperty(_GifPro, ":/res/acc/behind.gif");
	ui.labelAcc_4->setProperty(_GifPro, ":/res/acc/front.gif");
	ui.labelAcc_5->setProperty(_GifPro, ":/res/acc/left.gif");
	ui.labelAcc_6->setProperty(_GifPro, ":/res/acc/right.gif");
	connect(device, &DeviceControl::sigLogMessage, this, &CalibrationDialog::onDeviceMessage);
	connect(ui.btnAccFinished, &QAbstractButton::clicked, this, &CalibrationDialog::close);
	ui.stackedWidgetProgress->setCurrentIndex(calib);
	if (_Accelerometer == calib) {
		ui.widgetAccProgress->setVisible(true);
		m_pMovieAcc = new QMovie(ui.labelAcc_1->property(_GifPro).toString());
		ui.labelAccGif->setMovie(m_pMovieAcc);
		connect(m_pMovieAcc, &QMovie::finished, [this]() { 
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
	qDebug() << text;
	ui.textBrowser->append(QDateTime::currentDateTime().toString("[hh:mm:ss.zzz]:"));
	ui.textBrowser->append(text);
}

void CalibrationDialog::onDeviceMessage(QString data)
{
	if (data.isEmpty()) return;
	addLogToBrowser(data);
	if (m_pMovieAcc) m_pMovieAcc->stop();
	if (data.contains(_AccIng_)) {
		m_pMovieAcc->start();
		ui.stackedWidgetProgress->setCurrentIndex(_Accelerometer);
		//m_pMovieAcc->setFileName(":/res/acc/top.gif");
		m_pMovieAcc->setFileName(ui.labelAcc_1->property(_GifPro).toString());
	}
	else if (data.contains("Acc +z data get")) {
		//m_pMovieAcc->setFileName(":/res/acc/bottom.gif");
		ui.labelAcc_1->setEnabled(true);
	}
	else if (data.contains("Acc -z data get")) {
		//m_pMovieAcc->setFileName(":/res/acc/behind.gif");
		ui.labelAcc_2->setEnabled(true);
	}
	else if (data.contains("Acc -x data get")) {
		//m_pMovieAcc->setFileName(":/res/acc/front.gif");
		ui.labelAcc_3->setEnabled(true);
	}
	else if (data.contains("Acc +x data get")) {
		//m_pMovieAcc->setFileName(":/res/acc/left.gif");
		ui.labelAcc_4->setEnabled(true);
	}
	else if (data.contains("Acc -y data get")) {
		//m_pMovieAcc->setFileName(":/res/acc/right.gif");
		ui.labelAcc_5->setEnabled(true);
	}
	else if (data.contains("Acc +y data get")) {
		ui.labelAcc_6->setEnabled(true);		
	}
	else if (data.contains(_AccFinished_)) {
		m_pMovieAcc->stop();
		m_bAccFinished = true;
		ui.btnAccFinished->setVisible(true);
		_ShowInfoMessage(tr("加计校准完成"));
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
	if (ui.stackedWidgetProgress->currentIndex() == _Accelerometer && m_pMovieAcc) m_pMovieAcc->start();
}

void CalibrationDialog::hideEvent(QHideEvent* event)
{
	if (m_pLabelBackground) m_pLabelBackground->close();
}
