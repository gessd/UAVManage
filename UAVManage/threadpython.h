#pragma once

#include <QThread>
#include <Python.h>
#include "mypythondefine.h"

#define _WaypintFile_ "/pythonapi/waypoint.csv"
enum PythonRunState
{
	PythonRunNone = 0,
	PythonSuccessful,
	PythonError,
	PythonFileError,
	PythonTimeout,
	PythonCodeError,
	PythonWaypointError,
	PythonWaypointNull
};

class PythonToCplusplusClass : public QObject
{
	Q_OBJECT
public:
	//ʱ��
	static PyObject* Fly_Time(PyObject* self, PyObject* args);
	//����������Ϣ
	static PyObject* Fly_Waypoint(PyObject* self, PyObject* args);
	//�����е�������Ϣ
	static PyObject* Fly_Location(PyObject* self, PyObject* args);
	//�����е�ͣ��ʱ��
	static PyObject* Fly_hover(PyObject* self, PyObject* args);
	//�����е���ת�Ƕ�
	static PyObject* Fly_revolve(PyObject* self, PyObject* args);
	//���ٶȺ���
	static PyObject* Fly_speedWaypoint(PyObject* self, PyObject* args);
	//�����ٶ�
	static PyObject* Fly_setSpeed(PyObject* self, PyObject* args);
	//��ʼλ��
	static PyObject* Fly_StartLocation(PyObject* self, PyObject* args);
	//����LEDģʽ
	static PyObject* Fly_LedMode(PyObject* self, PyObject* args);
	//��������������
	static PyObject* Fly_Moveto(PyObject* self, PyObject* args);
	//���������
	static PyObject* Fly_MoveAddTo(PyObject* self, PyObject* args);
};

class ThreadPython : public QThread
{
	Q_OBJECT

public:
	ThreadPython(QObject *parent = NULL);
	~ThreadPython();
	bool compilePythonCode(QByteArray arrCode, bool bUpload = false);
	PythonRunState getLastState();
private:
	bool compilePythonFile(QString qstrFile, bool bUpload);
	QString checkWaypoint();
private:
	void run();
private:
	QString m_qstrFilePath;
	PythonRunState m_pythonState;
	bool m_bUpload;
};
