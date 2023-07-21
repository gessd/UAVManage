#pragma once

#include <QDialog>
#include <QSerialPort>
#include <QThread>
#include <QTimer>
#include "ui_deviceserial.h"
#include "serial/qextserialenumerator.h"
#include "Ymodem/YmodemFileTransmit.h"

class DeviceSerial : public QDialog
{
	Q_OBJECT

public:
	DeviceSerial(QWidget *parent = nullptr);
	~DeviceSerial();
	void updateSerial();
	bool isSerialEnabled();
public slots:
	void onSerialData(QByteArray data);
	void onBtnWrite();
	void onBtnRead();
	void onBtnSerial();
	void onDeviceDiscovered(const QextPortInfo& info);
	void onDeviceRemoved(const QextPortInfo& info);
	void onBtnCheckFirmware();
	void onBtnManualFirmware();
	void onLineEditChanged(QString text);
	void onTimeoutWrite();
	void onSerialReadyRead();
	void onBtnSendClicekd();
	void onBtnClearClicked();
	void onBtnAutoUpdateFirmwareClicked();

	//固件更新进度
	void  onYmodemTransmitProgress(int progress);
	//固件更新状态
	void onYmodemTransmitStatus(YmodemFileTransmit::Status status);

	void on_btnReadID_clicked();
protected:
	void showEvent(QShowEvent* event);
	void closeEvent(QCloseEvent* event);
	void hideEvent(QHideEvent* event);
	void keyPressEvent(QKeyEvent* event);
private:
	void sendDataToSerial(const QByteArray data);
	void dataRecord(bool send, QByteArray data);
signals:
	//void serialDataSend(const QByteArray data);
	void sigDeviceEnabled(bool enable);
private:
	Ui::DeviceSerial ui;
	QLabel* m_pLabelBackground;
	QSerialPort m_serialPort;
	//QThread m_thread;
	QextSerialEnumerator m_qextSerial;
	//SerialThread* m_pSerial;
	bool m_bWriteFinished;
	QTimer m_timeoutWrite;

	//固件更新
	YmodemFileTransmit* m_pYmodemFileTransmit;
	//固件正在更新中
	bool m_bYmodemTransmitStatus;
	//更新的固件文件
	QString m_qstrBinFile;
};
