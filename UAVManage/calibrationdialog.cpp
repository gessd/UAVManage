#include "calibrationdialog.h"
#include <QDateTime>
#include <QDebug>
#include <QTimer>

CalibrationDialog::CalibrationDialog(int calib, DeviceControl* device, QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Dialog);
	m_pLabelBackground = nullptr;
	m_pMovieAcc = nullptr;
	m_bAccFinished = false;
	//ui.btnAccFinished->setVisible(false);
	ui.widgetAccProgress->setVisible(false);
	connect(device, &DeviceControl::sigLogMessage, this, &CalibrationDialog::onDeviceMessage);
	connect(ui.btnAccFinished, &QAbstractButton::clicked, this, &CalibrationDialog::close);
	ui.stackedWidgetProgress->setCurrentIndex(calib);
	if (_Accelerometer == calib) {
		ui.widgetAccProgress->setVisible(true);
		m_pMovieAcc = new QMovie(":/res/gif/left.gif");
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
	addLogToBrowser(data);
	if (m_pMovieAcc) m_pMovieAcc->stop();
	if (data.contains(_AccIng_)) {
		m_pMovieAcc->start();
		ui.stackedWidgetProgress->setCurrentIndex(_Accelerometer);
	}
	else if (data.contains("Acc +z data get")) {
		m_pMovieAcc->setFileName(":/res/gif/bottom.gif");
		ui.labelAcc_1->setEnabled(true);
	}
	else if (data.contains("Acc -z data get")) {
		m_pMovieAcc->setFileName(":/res/gif/left.gif");
		ui.labelAcc_2->setEnabled(true);
	}
	else if (data.contains("Acc -x data get")) {
		m_pMovieAcc->setFileName(":/res/gif/right.gif");
		ui.labelAcc_3->setEnabled(true);
	}
	else if (data.contains("Acc +x data get")) {
		m_pMovieAcc->setFileName(":/res/gif/front.gif");
		ui.labelAcc_4->setEnabled(true);
	}
	else if (data.contains("Acc -y data get")) {
		m_pMovieAcc->setFileName(":/res/gif/behind.gif");
		ui.labelAcc_5->setEnabled(true);
	}
	else if (data.contains("Acc +y data get")) {
		ui.labelAcc_6->setEnabled(true);
		ui.btnAccFinished->setVisible(true);
		m_bAccFinished = true;
	}
	if (false == m_bAccFinished && m_pMovieAcc) m_pMovieAcc->start();
}

void CalibrationDialog::showEvent(QShowEvent* event)
{
	if (m_pLabelBackground) {
		delete m_pLabelBackground;
		m_pLabelBackground = nullptr;
	}
	m_pLabelBackground = new QLabel(dynamic_cast<QWidget*>(parent()));
	m_pLabelBackground->setStyleSheet(QString("background-color: rgba(0, 0, 0, 50%);"));
	m_pLabelBackground->setFixedSize(dynamic_cast<QWidget*>(parent())->size());
	m_pLabelBackground->show();
	if (ui.stackedWidgetProgress->currentIndex() == _Accelerometer && m_pMovieAcc) m_pMovieAcc->start();
}

void CalibrationDialog::hideEvent(QHideEvent* event)
{
	if (m_pLabelBackground) m_pLabelBackground->close();
}
