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
	//时间
	static PyObject* Fly_Time(PyObject* self, PyObject* args);
	//完整航点信息
	static PyObject* Fly_Waypoint(PyObject* self, PyObject* args);
	//航点中的坐标信息
	static PyObject* Fly_Location(PyObject* self, PyObject* args);
	//航点中的停留时间
	static PyObject* Fly_hover(PyObject* self, PyObject* args);
	//航点中的旋转角度
	static PyObject* Fly_revolve(PyObject* self, PyObject* args);
	//带速度航点
	static PyObject* Fly_speedWaypoint(PyObject* self, PyObject* args);
	//设置速度
	static PyObject* Fly_setSpeed(PyObject* self, PyObject* args);
	//初始位置
	static PyObject* Fly_StartLocation(PyObject* self, PyObject* args);
	//设置LED模式
	static PyObject* Fly_LedMode(PyObject* self, PyObject* args);
	//单方向增量飞行
	static PyObject* Fly_Moveto(PyObject* self, PyObject* args);
	//单方向飞行
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
