#pragma once

#include <QWidget>
#include "ui_PlaceInfoDialog.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QShowEvent>
#include <QHideEvent>
#include <QCloseEvent>
#include <QThread>
#include <QTimer>

//NLINK设置协议格式
enum _NLINK_Setting_Frame0_
{
	_Header = 0,
	_Mark,
	_mix,	     //0x00写入,0x01读取
	_role,
	_reserved1,
	_id = _reserved1 + 5,
	_reserved2,
	_group = _reserved2+26,
	_xyz0,
	_xyz1 =     _xyz0 + 9,
	_xyz2 =     _xyz1 + 9,
	_xyz3 =     _xyz2 + 9,
	_xyz4 =     _xyz3 + 9,
	_xyz5 =     _xyz4 + 9,
	_xyz6 =     _xyz5 + 9,
	_xyz7 =     _xyz6 + 9,
	_xyz8 =     _xyz7 + 9,
	_xyz9 =     _xyz8 + 9,
	_Checksum = _xyz9+9,
	_ByteCount   //完整数据长度为128
};

#define _Get_Setting_Frame0_ QByteArray::fromHex("54000100000000000000000000FF00FF0000FFFF00FFFF0000000000000000FFFFFFFFFF000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000004A")
#define _OneKey_Set_Start_   QByteArray::fromHex("54000200000000000000000000FF00FF0008FFFF00FFFF0000000000000000FFFFFFFFFF0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000053")
#define _OneKey_Set_Next_    QByteArray::fromHex("54002000000000000000000000FF00FF0000FFFF00FFFF0000000000000000FFFFFFFFFF0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000069")
//基站无效数据
#define _InvalidValue_   -8388

class SerialWorker : public QObject
{
	Q_OBJECT
public:
	explicit SerialWorker(QSerialPort* ser, QObject* parent = nullptr);
signals:
	void sendResultToGui(QByteArray result);
public slots:
	void onDataSendWork(const QByteArray data);
	void onDataReciveWork();
private:
	QSerialPort* m_pSerial;
	QByteArray m_SerialData;
};

class PlaceInfoDialog : public QDialog
{
	Q_OBJECT

public:
	PlaceInfoDialog(QPoint place, QWidget *parent);
	~PlaceInfoDialog();
	QMap<QString, QPoint> getStationAddress();
	bool isValidStation();
protected:
	void showEvent(QShowEvent* event);
	void closeEvent(QCloseEvent* event);
	void hideEvent(QHideEvent* event);
private slots:
	void onBtnOpenClicked();
	void onBtnReadClicked();
	void onBtnWriteClicked();
	void onBtnOnekeyClicked();
	void onParseSettingFrame(QByteArray arrNLINKData);
	void onComparePlace(QPoint point);
	void onTimerOnekeyStatus();
signals:
	void serialDataSend(const QByteArray data);
private:
	Ui::PlaceInfoDialog ui;
	QSerialPort m_serialPort;
	bool m_bSetNLINK;
	bool m_bOnekeySetNLINK;
	int m_nOnekeySetIndex;
	//基站标定状态 [0未标定|1标定成功|-1标定失败]
	int m_stationStatus;
	QThread m_threadSerial;
	QLabel* m_pLabelBackground;
	QPoint m_pointPlace;
	QByteArray m_arrLastData;
	QChar m_cOneKeyStatus;
	QTimer m_timerOnekeyStatus;
};
