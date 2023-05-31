#pragma once

#include <QDialog>
#include <QSerialPort>
#include <QThread>
#include <QTimer>
#include "ui_deviceserial.h"
#include "serial/qextserialenumerator.h"

class SerialThread : public QObject
{
	Q_OBJECT
public:
	explicit SerialThread(QSerialPort* ser, QObject* parent = nullptr);
	void clear();
signals:
	void sendResultToGui(QByteArray result);
public slots:
	void onDataSendWork(const QByteArray data);
	void onDataReciveWork();
private:
	QSerialPort* m_pSerial;
	QString m_qstrData;
};

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
protected:
	void showEvent(QShowEvent* event);
	void closeEvent(QCloseEvent* event);
	void hideEvent(QHideEvent* event);
signals:
	void serialDataSend(const QByteArray data);
	void sigDeviceEnabled(bool enable);
private:
	Ui::DeviceSerial ui;
	QLabel* m_pLabelBackground;
	QSerialPort m_serialPort;
	QThread m_thread;
	QextSerialEnumerator m_qextSerial;
	SerialThread* m_pSerial;
	bool m_bWriteFinished;
	QTimer m_timeoutWrite;
};
