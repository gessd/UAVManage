#include "threadpython.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include "messagelistdialog.h"

struct _MarkPoint
{
	unsigned int x;
	unsigned int y;
	unsigned int z;
	_MarkPoint() {
		x = y = z = 0;
	}
	_MarkPoint(unsigned int n1, unsigned int n2, unsigned int n3) {
		x = n1;
		y = n2;
		z = n3;
	}
}; 

//使用全局变量，便于python数据交互
unsigned int g_nSpaceX = 0;
unsigned int g_nSpaceY = 0;
QString g_deviceName;
QVector<NavWayPointData> g_waypointData;

//预设标定点
//标定点名称，空间坐标
QMap<QString, _MarkPoint> g_mapMarkPoint;

void QZAPI::showWaypointError(QString error)
{
	QString text = QString("%1 第%2步 %3").arg(g_deviceName).arg(g_waypointData.count()).arg(error);
	_ShowErrorMessage(text);
}

QZAPI QZAPI::m_qzaip;
////Python to c++ data start////
//python接口格式，注册c++函数到python中，便于通过python调用c++函数
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
	{"FlyToMarkPoint",	QZAPI::FlyToMarkPoint,		METH_VARARGS, "FlyToMarkPoint"},
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
	//注册python回调，用于python相互调用
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
	data.x = x;
	data.y = y;
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
	data.x = x;
	data.y = y;
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
	data.x = x;
	data.y = y;
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
	case 1: data.x = s; break;
	case 2: data.y = s; break;
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
	case 1: data.x += s; break;
	case 2: data.x -= s; break;
	case 3: data.y += s; break;
	case 4: data.y -= s; break;
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
		QZAPI::Instance()->showWaypointError(tr("标定点参数值错误"));
		return nullptr;
	}
	if (NULL == name) {
		QZAPI::Instance()->showWaypointError(tr("标定点名称值错误"));
		return nullptr;
	}
	//判断是否飞出场地范围
	if (x <= 0 || y <= 0 || x >= g_nSpaceX || y >= g_nSpaceY) {
		QZAPI::Instance()->showWaypointError(tr("标定点超出场地范围"));
		return nullptr;
	}
	QString qstrName(name);
	qDebug() << "添加标定点" << qstrName << x << y << z;
	if (g_mapMarkPoint.contains(qstrName)) {
		QZAPI::Instance()->showWaypointError(tr("存在相同名称标定点")+qstrName);
		return nullptr;
	}
	g_mapMarkPoint.insert(qstrName, _MarkPoint(x,y,z));
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlySetSpeed(PyObject* self, PyObject* args)
{
	int n = 0;
	if (!PyArg_ParseTuple(args, "i", &n)) {
		QZAPI::Instance()->showWaypointError(tr("速度参数值错误"));
		return nullptr;
	}
	qDebug() << "设置飞行速度" << n;
	NavWayPointData last = g_waypointData.back();
	NavWayPointData data;
	data.x = last.x;
	data.y = last.y;
	data.z = last.z;
	data.param1 = n;
	data.commandID = _WaypointSpeed;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlySetLed(PyObject* self, PyObject* args)
{
	int n = 0;
	if (!PyArg_ParseTuple(args, "i", &n)) {
		QZAPI::Instance()->showWaypointError(tr("LED灯参数值错误"));
		return nullptr;
	}
	qDebug() << "设置LED灯模式" << n;
	NavWayPointData last = g_waypointData.back();
	NavWayPointData data;
	data.x = last.x;
	data.y = last.y;
	data.z = last.z;
	data.param1 = n;
	data.commandID = _WaypointLed;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyHover(PyObject* self, PyObject* args)
{
	int n = 0;
	if (!PyArg_ParseTuple(args, "i", &n)) {
		QZAPI::Instance()->showWaypointError(tr("悬停参数值错误"));
		return nullptr;
	}
	qDebug() << "悬停时间" << n << "毫秒";
	NavWayPointData last = g_waypointData.back();
	if (last.z <= 0.0) {
		//判断是否已经飞离地面
		QZAPI::Instance()->showWaypointError(tr("没有起飞无法悬停"));
		return nullptr;
	}
	//悬停时间精确到毫秒
	NavWayPointData data;
	data.x = last.x;
	data.y = last.y;
	data.z = last.z;
	data.param1 = n / 1000.0;
	data.commandID = _WaypointHover;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyTakeoff(PyObject* self, PyObject* args)
{
	int n = 0;
	if (!PyArg_ParseTuple(args, "i", &n)) {
		QZAPI::Instance()->showWaypointError(tr("起飞参数值错误"));
		return nullptr;
	}
	qDebug() << "起飞高度" << n;
	NavWayPointData lastWaypoint = g_waypointData.back();
	if (_WaypointStart != lastWaypoint.commandID) {
		QZAPI::Instance()->showWaypointError(tr("起飞必须是第一步"));
		return nullptr;
	}
	NavWayPointData data;
	data.x = lastWaypoint.x;
	data.y = lastWaypoint.y;
	data.z = lastWaypoint.z + n;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyLand(PyObject* self, PyObject* args)
{
	qDebug() << "降落";
	NavWayPointData lastWaypoint = g_waypointData.back();
	if (0 >= lastWaypoint.z) {
		QZAPI::Instance()->showWaypointError(tr("高度已降为0"));
		return nullptr;
	}
	NavWayPointData data;
	data.x = lastWaypoint.x;
	data.y = lastWaypoint.y;
	data.z = 0;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyTimeGroup(PyObject* self, PyObject* args)
{
	self;
	int m, s;
	m = s = 0;
	if (!PyArg_ParseTuple(args, "i|i", &m, &s)) {
		QZAPI::Instance()->showWaypointError(tr("时间参数值错误"));
		return nullptr;
	}
	qDebug() << "时间组范围" << m << s;
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyRevolve(PyObject* self, PyObject* args)
{
	float angle = 0.0;
	if (!PyArg_ParseTuple(args, "f", &angle)) {
		QZAPI::Instance()->showWaypointError(tr("旋转参数值错误"));
		return nullptr;
	}
	qDebug() << "无人机旋转" << angle;
	NavWayPointData last = g_waypointData.back();
	NavWayPointData data;
	data.x = last.x;
	data.y = last.y;
	data.z = last.z;
	data.param1 = angle;
	data.commandID = _WaypointRevolve;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyTo(PyObject* self, PyObject* args)
{
	int x, y, z;
	x = y = z = 0;
	if (!PyArg_ParseTuple(args, "i|i|i", &x, &y, &z)) {
		QZAPI::Instance()->showWaypointError(tr("飞行到参数值错误"));
		return nullptr;
	}
	qDebug() << "飞行到绝对位置" << x << y << z;
	NavWayPointData data;
	data.x = x;
	data.y = y;
	data.z = z;
	//判断是否飞出场地范围
	if (data.x <= 0 || data.y <= 0 || data.x >= g_nSpaceX || data.y >= g_nSpaceY) {
		QZAPI::Instance()->showWaypointError(tr("飞出场地范围"));
		return nullptr;
	}
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyMove(PyObject* self, PyObject* args)
{
	int direction, n;
	direction = n = 0;
	if (!PyArg_ParseTuple(args, "i|i", &direction, &n)) {
		QZAPI::Instance()->showWaypointError(tr("向固定方向飞行参数值错误"));
		return nullptr;
	}
	qDebug() << "相对移动" << direction << n;
	NavWayPointData lastWaypoint = g_waypointData.back();
	NavWayPointData data;
	data.x = lastWaypoint.x;
	data.y = lastWaypoint.y;
	data.z = lastWaypoint.z;
	//["前", "1"] ,
	//["后", "2"],
	//["右", "3"],
	//["左", "4"],
	//["上", "5"],
	//["下", "6"],
	switch (direction)
	{
	case 1: data.x += n; break;
	case 2: data.x -= n; break;
	case 3: data.y += n; break;
	case 4: data.y -= n; break;
	case 5: data.z += n; break;
	case 6: data.z -= n; break;
	}
	//判断是否飞出场地范围
	if (data.x <= 0 || data.y <= 0 || data.x >= g_nSpaceX || data.y >= g_nSpaceY) {
		QZAPI::Instance()->showWaypointError(tr("飞出场地范围"));
		return nullptr;
	}
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlyToMarkPoint(PyObject* self, PyObject* args)
{
	char* name = NULL;
	if (!PyArg_ParseTuple(args, "s", &name)) {
		QZAPI::Instance()->showWaypointError(tr("飞到标定点参数值错误"));
		return nullptr;
	}
	if (NULL == name) {
		QZAPI::Instance()->showWaypointError(tr("飞到标定点名称错误"));
		return nullptr;
	}
	QString qstrName(name);
	if (false == g_mapMarkPoint.contains(qstrName)) {
		QZAPI::Instance()->showWaypointError(tr("飞到标定点名称不存在"));
		return nullptr;
	}
	_MarkPoint point = g_mapMarkPoint[qstrName];
	qDebug() << "飞行到标定点" << qstrName << point.x << point.y << point.z;
	NavWayPointData data;
	data.x = point.x;
	data.y = point.y;
	data.z = point.z;
	data.commandID = _WaypointFly;
	g_waypointData.append(data);
	return Py_BuildValue("i", 0);
}

QZAPI* QZAPI::Instance()
{
	return &m_qzaip;
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

void ThreadPython::initParam(unsigned int nSpaceX, unsigned int nSapaceY, QString name, unsigned int nStartX, unsigned int nStartY)
{
	//因为是全局变量，使用前清空内容
	qDebug() << "建立航点数据" << name;
	g_waypointData.clear();
	g_mapMarkPoint.clear();
	g_nSpaceX = nSpaceX;
	g_nSpaceY = nSapaceY;
	g_deviceName = name;
	//舞步前添加起始位置
	NavWayPointData startLocation;
	startLocation.x = nStartX;
	startLocation.y = nStartY;
	startLocation.commandID = _WaypointStart;
	g_waypointData.append(startLocation);
}

bool ThreadPython::compilePythonCode(QByteArray arrCode)
{
	if(arrCode.isEmpty()) return false;
	m_pythonState = PythonRunNone;
	////python代码添加必要头部数据
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

	//python代码保存至文件后执行
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
		qDebug() << "python运行库加载失败";
		return false;
	}
	//设置python运行环境目录
	QString qstrPythonRunPath = QApplication::applicationDirPath() + _PyDllPath_;
	//TODO检查python必要文件是否存在，防止执行python接口引起程序崩溃
	QDir dirPython(qstrPythonRunPath);
	if (false == dirPython.exists()) {
		qDebug() << "python文件夹不存在";
		return false;
	}
	std::wstring wstrpath = qstrPythonRunPath.toStdWString();
	Py_SetPythonHome((wchar_t*)wstrpath.c_str());
	if (!QFile::exists(QApplication::applicationDirPath() + _PyNeceFie_)) {
		qDebug() << "python缺少必要文件";
		return false;
	}
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
	qDebug() << "python文件开始执行" << m_qstrFilePath;
	int n = PyRun_SimpleString("import sys");
	//QString qstrPythonApiPath = QString("sys.path.append('%1')").arg(QApplication::applicationDirPath() + _PythonApiFilePath_);
	QString qstrPythonPath = QString("sys.path.append('%1')").arg(QApplication::applicationDirPath());
	//设置python执行文件路径
	//PyRun_SimpleString(qstrPythonApiPath.toStdString().c_str());
	PyRun_SimpleString(qstrPythonPath.toStdString().c_str());
	//执行python文件,生成航点列表
	PyObject* pPyFile = PyImport_ImportModule(_PyFileName_);
	if (!pPyFile) {
		PyErr_CheckSignals();
		qDebug() << "python执行出错";
		Py_Finalize();
		m_pythonState = PythonCodeError;
		return;
	}
	//执行完成后释放python资源，否则下次无法执行
	Py_DECREF(pPyFile);
	Py_Finalize();
	m_pythonState = PythonSuccessful;
}
