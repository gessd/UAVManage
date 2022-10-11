#include "threadpython.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMessageBox>

QVector<NavWayPointData> g_waypointData;
////Python to c++ data start////
PyMethodDef xWrapMethods[] = {
	{ "Fly_Time", PythonToCplusplusClass::Fly_Time, METH_VARARGS, "Fly_Time" },
	{ "Fly_Waypoint", PythonToCplusplusClass::Fly_Waypoint, METH_VARARGS, "Fly_Waypoint" },
	{ "Fly_Location", PythonToCplusplusClass::Fly_Location, METH_VARARGS, "Fly_Location" },
	{ "Fly_hover",     PythonToCplusplusClass::Fly_hover, METH_VARARGS, "Fly_hover" },
	{ "Fly_revolve",  PythonToCplusplusClass::Fly_revolve, METH_VARARGS, "Fly_revolve" },
	{ "Fly_speedWaypoint", PythonToCplusplusClass::Fly_speedWaypoint, METH_VARARGS, "Fly_speedWaypoint" },
	{ "Fly_setSpeed", PythonToCplusplusClass::Fly_setSpeed, METH_VARARGS, "Fly_setSpeed" },
	{ "Fly_StartLocation", PythonToCplusplusClass::Fly_StartLocation, METH_VARARGS, "Fly_StartLocation" },
	{ "Fly_LedMode", PythonToCplusplusClass::Fly_LedMode, METH_VARARGS, "Fly_LedMode" },
	{ "Fly_Moveto", PythonToCplusplusClass::Fly_Moveto, METH_VARARGS, "Fly_Moveto" },
	{ "Fly_MoveAddTo", PythonToCplusplusClass::Fly_MoveAddTo, METH_VARARGS, "Fly_MoveAddTo" },
	{ NULL, NULL, 0, NULL }
};
static struct PyModuleDef BicubicModule = {
	PyModuleDef_HEAD_INIT,
	"PythonToCplusplusClass",
	NULL,
	-1,
	xWrapMethods
};

PyMODINIT_FUNC PyInit_PythonCallBack(void)
{
	PyObject* module;
	module = PyModule_Create(&BicubicModule);
	return module;
}
PyObject* PythonToCplusplusClass::Fly_Time(PyObject* self, PyObject* args)
{
	char* t = NULL;
	if (!PyArg_ParseTuple(args, "s", &t)) {
		return Py_BuildValue("i", 0);
	}
	QString qstrTime(t);
	QStringList list = qstrTime.split(":");
	if(2 != list.count()) return Py_BuildValue("i", 0);
	NavWayPointData data;
	data.param1 = list[0].toInt();
	data.param2 = list[1].toInt();
	data.commandID = 31005;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* PythonToCplusplusClass::Fly_Waypoint(PyObject* self, PyObject* args)
{
	float x, y, z, h, a, p, w;
	if (!PyArg_ParseTuple(args, "f|f|f|f|f|f|f", &x, &y, &z, & h, &a, & p, &w)) {
		return Py_BuildValue("i", 0);
	}
	if (a < 0.2) a = 0.2;
	NavWayPointData data;
	data.x = x * 1000;
	data.y = y * 1000;
	data.z = z;
	data.param1 = h;
	data.param2 = a;
	data.param3 = p;
	data.param4 = w;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* PythonToCplusplusClass::Fly_Location(PyObject* self, PyObject* args)
{
	float x, y, z;
	if (!PyArg_ParseTuple(args, "f|f|f", &x, &y, &z)) {
		return Py_BuildValue("i", 0);
	}
	NavWayPointData data;
	data.x = x * 1000;
	data.y = y * 1000;
	data.z = z;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* PythonToCplusplusClass::Fly_hover(PyObject* self, PyObject* args)
{
	float hold;
	if (!PyArg_ParseTuple(args, "f", &hold)) {
		return Py_BuildValue("i", 0);
	}
	if (g_waypointData.isEmpty()) {
		//TODO
		//没有航点信息无法设置停留时间,需要提示,此处为子线程无法使用弹窗
		//return Py_BuildValue("i", 0);
	}
	NavWayPointData data = g_waypointData.back();
	data.param1 = hold;
	data.param4 = 0;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* PythonToCplusplusClass::Fly_revolve(PyObject* self, PyObject* args)
{
	float yaw;
	if (!PyArg_ParseTuple(args, "f", &yaw)) {
		return Py_BuildValue("i", 0);
	}
	if (g_waypointData.isEmpty()) {
		//TODO
		//没有航点信息无法设置偏转角度,需要提示,此处为子线程无法使用弹窗
		//return Py_BuildValue("i", 0);
	}
	NavWayPointData data = g_waypointData.back();
	data.param1 = 0;
	data.param4 = yaw;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* PythonToCplusplusClass::Fly_speedWaypoint(PyObject* self, PyObject* args)
{
	//带速度航点
	float x, y, z, s;
	if (!PyArg_ParseTuple(args, "f|f|f|f", &x, &y, &z, &s)) {
		return Py_BuildValue("i", 0);
	}
	NavWayPointData data;
	data.x = x * 1000;
	data.y = y * 1000;
	data.z = z;
	data.param1 = s;
	data.commandID = 31000;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* PythonToCplusplusClass::Fly_setSpeed(PyObject* self, PyObject* args)
{
	float s;
	if (!PyArg_ParseTuple(args, "f", &s)) {
		return Py_BuildValue("i", 0);
	}
	//设置航点速度
	//TODO 航点位置使用上一次位置
	NavWayPointData data = g_waypointData.back();
	data.param1 = s;
	data.param3 = 0;
	data.param4 = 4;
	data.commandID = 31000;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* PythonToCplusplusClass::Fly_StartLocation(PyObject* self, PyObject* args)
{
	float x, y;
	if (!PyArg_ParseTuple(args, "f|f", &x, &y)) {
		return Py_BuildValue("i", 0);
	}
	NavWayPointData data;
	data.x = x * 1000;
	data.y = y * 1000;
	data.commandID = 31004;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* PythonToCplusplusClass::Fly_LedMode(PyObject* self, PyObject* args)
{
	int m;
	if (!PyArg_ParseTuple(args, "n", &m)) {
		return Py_BuildValue("i", 0);
	}
	NavWayPointData data;
	data.param1 = m;
	data.commandID = 31003;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}
PyObject* PythonToCplusplusClass::Fly_Moveto(PyObject* self, PyObject* args)
{
	float s;
	int d;
	if (!PyArg_ParseTuple(args, "n|f", &d, &s)) {
		return Py_BuildValue("i", 0);
	}
	NavWayPointData data = g_waypointData.back();
	switch (d)
	{
	case 1: data.x = s * 1000; break;
	case 2: data.y = s * 1000; break;
	case 3: data.z = s; break;
	}
	data.param1 = 0; 
	data.param4 = 0;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* PythonToCplusplusClass::Fly_MoveAddTo(PyObject* self, PyObject* args)
{
	float s;
	int d;
	if (!PyArg_ParseTuple(args, "n|f", &d, &s)) {
		return Py_BuildValue("i", 0);
	}
	NavWayPointData data = g_waypointData.back();
	//TODO
	//需要判断是否有效航点,commandID=16;
	switch (d)
	{
	case 1: data.x += s * 1000; break;
	case 2: data.x -= s * 1000; break;
	case 3: data.y += s * 1000; break;
	case 4: data.y -= s * 1000; break;
	case 5: data.z += s; break;
	case 6: data.z -= s; break;
	}
	data.param1 = 0;
	data.param4 = 0;
	data.commandID = 16;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

////Python to c++ data end////

ThreadPython::ThreadPython(QObject *parent)
	: QThread(parent)
{
	m_pythonState = PythonRunNone;
}

ThreadPython::~ThreadPython()
{
	if (isRunning()) {
		quit();
		wait();
	}
}

bool ThreadPython::compilePythonCode(QByteArray arrCode)
{
	g_waypointData.clear();
	if(arrCode.isEmpty()) return false;
	m_pythonState = PythonRunNone;
	//保存至文件后执行
	QString qstrHeadFile = QApplication::applicationDirPath() + _PyHeadFile_;
	if (!QFile::exists(qstrHeadFile)) return false;
	QFile fileHead(qstrHeadFile);
	if (!fileHead.open(QIODevice::ReadOnly)) return false;
	QByteArray arrHead = fileHead.readAll();
	if (arrHead.isEmpty()) {
		fileHead.close();
		return false;
	}
	arrCode.prepend(arrHead+"\r\n");
	fileHead.close();

	QString qstrRunFile = QApplication::applicationDirPath() + _PyRunFilePath_;
	QFile filePython(qstrRunFile);
	if (!filePython.open(QIODevice::ReadWrite | QIODevice::Truncate)) return false;
	int len = filePython.write(arrCode);
	filePython.flush();
	if (0 >= len) {
		filePython.close();
		return false;
	}
	filePython.close();
	return compilePythonFile(qstrRunFile);
}

PythonRunState ThreadPython::getLastState()
{
	return m_pythonState;
}

QVector<NavWayPointData> ThreadPython::getWaypointData()
{
	return g_waypointData;
}

bool ThreadPython::compilePythonFile(QString qstrFile)
{
	if (!QFile::exists(qstrFile)) return false;
	m_qstrFilePath = qstrFile;
	//添加接口类
	if (PyImport_AppendInittab("PythonToCplusplusClass", PyInit_PythonCallBack) == -1) {
		qDebug() << "----init callback error";
		return false;
	}
	//设置python运行环境目录
	QString qstrPythonRunPath = QApplication::applicationDirPath() + _PyDllPath_;
	//TODO检查python必要文件是否存在，防止执行python接口引起程序崩溃
	QDir dirPython(qstrPythonRunPath);
	if (false == dirPython.exists()) return false;
	std::wstring wstrpath = qstrPythonRunPath.toStdWString();
	Py_SetPythonHome((wchar_t*)wstrpath.c_str());
	if (!QFile::exists(QApplication::applicationDirPath() + _PyNeceFie_)) return false;
	//初始化python环境
	if (!Py_IsInitialized()) Py_Initialize();
	if (!Py_IsInitialized()) return false;
	start();
	return true;
}

QString ThreadPython::checkWaypoint()
{
	//1开始必须是起始位置，有且只有一条
	return "";
}

void ThreadPython::run()
{
	if (!QFile::exists(m_qstrFilePath)) return;
	int n = PyRun_SimpleString("import sys");
	QString qstrPythonApiPath = QString("sys.path.append('%1')").arg(QApplication::applicationDirPath() + _PythonApiFilePath_);
	QString qstrPythonPath = QString("sys.path.append('%1')").arg(QApplication::applicationDirPath());
	//设置python执行文件路径
	PyRun_SimpleString(qstrPythonApiPath.toStdString().c_str());
	PyRun_SimpleString(qstrPythonPath.toStdString().c_str());
	//执行python文件,生成航点列表
	PyObject* pPyFile = PyImport_ImportModule(_PyFileName_);
	if (!pPyFile) {
		qDebug() << "---- python error:";
		Py_Finalize();
		m_pythonState = PythonCodeError;
		return;
	}
	//执行完成后释放python资源，否则下次无法执行
	Py_DECREF(pPyFile);
	Py_Finalize();
	m_pythonState = PythonSuccessful;
}
