#include "threadpython.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMessageBox>

struct _SpacePoint
{
	int x;
	int y;
	int z;
	_SpacePoint() {
		x = y = z = 0;
	}
}; 
QVector<NavWayPointData> g_waypointData;
QMap<QString, _SpacePoint> g_mapMarkPoint;
////Python to c++ data start////
PyMethodDef xWrapMethods[] = {	
	{"FlyAddMarkPoint", QZAPI::FlyAddMarkPoint,		METH_VARARGS, "FlyAddMarkPoint"},
	{"FlySetSpeed",		QZAPI::FlySetSpeed,			METH_VARARGS, "FlySetSpeed"},
	{"FlySetLed",		QZAPI::FlySetLed,			METH_VARARGS, "FlySetLed"},
	{"FlyHover",		QZAPI::FlyHover,			METH_VARARGS, "FlyHover"},
	{"FlyTakeoff",		QZAPI::FlyTakeoff,			METH_VARARGS, "FlyTakeoff"},
	{"FlyLand",			QZAPI::FlyLand,				METH_VARARGS, "FlyLand"},
	{"FlyTimeGroup",	QZAPI::FlyTimeGroup,		METH_VARARGS, "FlyTimeGroup"},
	{"FlyRevolve",		QZAPI::FlyRevolve,			METH_VARARGS, "FlyRevolve"},
	{"FlyTo",			QZAPI::FlyTo,				METH_VARARGS, "FlyTo"},
	{"FlyMove",			QZAPI::FlyMove,				METH_VARARGS, "FlyMove"},
	{"FlyToPoint",		QZAPI::FlyToPoint,			METH_VARARGS, "FlyToPoint"},
	{ NULL, NULL, 0, NULL }
};
static struct PyModuleDef BicubicModule = {
	PyModuleDef_HEAD_INIT,
	"QZAPI",
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
PyObject* QZAPI::Fly_Time(PyObject* self, PyObject* args)
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

PyObject* QZAPI::Fly_Waypoint(PyObject* self, PyObject* args)
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

PyObject* QZAPI::Fly_Location(PyObject* self, PyObject* args)
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

PyObject* QZAPI::Fly_hover(PyObject* self, PyObject* args)
{
	float hold;
	if (!PyArg_ParseTuple(args, "f", &hold)) {
		return Py_BuildValue("i", 0);
	}
	if (g_waypointData.isEmpty()) {
		//TODO 没有航点信息无法设置停留时间,需要提示,此处为子线程无法使用弹窗
		//return Py_BuildValue("i", 0);
	}
	NavWayPointData data = g_waypointData.back();
	data.param1 = hold;
	data.param4 = 0;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::Fly_revolve(PyObject* self, PyObject* args)
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

PyObject* QZAPI::Fly_speedWaypoint(PyObject* self, PyObject* args)
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

PyObject* QZAPI::Fly_setSpeed(PyObject* self, PyObject* args)
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
	data.param4 = 0;
	data.commandID = 31000;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::Fly_LedMode(PyObject* self, PyObject* args)
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
PyObject* QZAPI::Fly_Moveto(PyObject* self, PyObject* args)
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

PyObject* QZAPI::Fly_MoveAddTo(PyObject* self, PyObject* args)
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

PyObject* QZAPI::FlyAddMarkPoint(PyObject* self, PyObject* args)
{
	char* name = NULL;
	int x, y, z;
	x = y = z = 0;
	if (!PyArg_ParseTuple(args, "s|i|i|i", &name, &x, &y, &z)) {
		return Py_BuildValue("i", 0);
	}
	if (NULL == name) return Py_BuildValue("i", 0);
	//TODO 判断标定点名称或位置重复
	QString qstrName(name);
	qDebug() << "添加标定点" << qstrName << x << y << z;
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlySetSpeed(PyObject* self, PyObject* args)
{
	int n = 0;
	if (!PyArg_ParseTuple(args, "i", &n)) {
		return Py_BuildValue("i", 0);
	}
	qDebug() << "设置飞行速度" << n;
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlySetLed(PyObject* self, PyObject* args)
{
	int n = 0;
	if (!PyArg_ParseTuple(args, "i", &n)) {
		return Py_BuildValue("i", 0);
	}
	qDebug() << "设置LED灯模式" << n;
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyHover(PyObject* self, PyObject* args)
{
	int n = 0;
	if (!PyArg_ParseTuple(args, "i", &n)) {
		return Py_BuildValue("i", 0);
	}
	qDebug() << "悬停时间" << n;
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyTakeoff(PyObject* self, PyObject* args)
{
	int n = 0;
	if (!PyArg_ParseTuple(args, "i", &n)) {
		return Py_BuildValue("i", 0);
	}
	qDebug() << "起飞高度" << n;
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyLand(PyObject* self, PyObject* args)
{
	qDebug() << "降落";
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyTimeGroup(PyObject* self, PyObject* args)
{
	self;
	int m, s;
	m = s = 0;
	if (!PyArg_ParseTuple(args, "i|i", &m, &s)) {
		return Py_BuildValue("i", 0);
	}
	qDebug() << "时间组范围" << m << s;
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyRevolve(PyObject* self, PyObject* args)
{
	float angle = 0.0;
	if (!PyArg_ParseTuple(args, "f", &angle)) {
		return Py_BuildValue("i", 0);
	}
	qDebug() << "无人机旋转" << angle;
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyTo(PyObject* self, PyObject* args)
{
	int x, y, z;
	x = y = z = 0;
	if (!PyArg_ParseTuple(args, "i|i|i", &x, &y, &z)) {
		return Py_BuildValue("i", 0);
	}
	qDebug() << "飞行到绝对位置" << x << y << z;
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyMove(PyObject* self, PyObject* args)
{
	int direction, n;
	direction = n = 0;
	if (!PyArg_ParseTuple(args, "i|i", &direction, &n)) {
		return Py_BuildValue("i", 0);
	}
	qDebug() << "相对移动" << direction << n;
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyToPoint(PyObject* self, PyObject* args)
{
	char* name = NULL;
	if (!PyArg_ParseTuple(args, "s", &name)) {
		return Py_BuildValue("i", 0);
	}
	if(NULL == name) return Py_BuildValue("i", 0);
	QString qstrName(name);
	qDebug() << "飞行到标定点" << name;
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
	////保存至文件后执行
	//QString qstrHeadFile = QApplication::applicationDirPath() + _PyHeadFile_;
	//if (!QFile::exists(qstrHeadFile)) return false;
	//QFile fileHead(qstrHeadFile);
	//if (!fileHead.open(QIODevice::ReadOnly)) return false;
	//QByteArray arrHead = fileHead.readAll();
	//if (arrHead.isEmpty()) {
	//	fileHead.close();
	//	return false;
	//}
	//arrCode.prepend(arrHead + "\r\n");
	//fileHead.close();

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
	if (PyImport_AppendInittab("QZAPI", PyInit_PythonCallBack) == -1) {
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
	//QString qstrPythonApiPath = QString("sys.path.append('%1')").arg(QApplication::applicationDirPath() + _PythonApiFilePath_);
	QString qstrPythonPath = QString("sys.path.append('%1')").arg(QApplication::applicationDirPath());
	//设置python执行文件路径
	//PyRun_SimpleString(qstrPythonApiPath.toStdString().c_str());
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
