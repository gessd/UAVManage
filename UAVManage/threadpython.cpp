#include "threadpython.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QtMath>
#include <QMessageBox>
#include "messagelistdialog.h"

bool m_bBlocklyError;
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
//当前设备名称
QString g_deviceName;
QVector<NavWayPointData> g_waypointData;
//用于区别积木块与python代码
bool g_bBlockly = true;		

//预设标定点
//标定点名称，空间坐标
QMap<QString, _MarkPoint> g_mapMarkPoint;
//时间组
QMap<QString, unsigned int> g_mapTimeGroup;
//动作累计时间 秒
unsigned int g_nTimeTotal;
//标记是否降落
bool g_bLand;

//计算两点间距离
int getDistance(int x1, int y1, int z1, int x2, int y2, int z2)
{
	int d = qSqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) + (z1 - z2) * (z1 - z2));
	return d;
}
PyObject* QZAPI::examineWaypoint()
{
	if (g_bLand) {
		showWaypointError("降落后不能有其他动作");
		return nullptr;
	}
	NavWayPointData data = g_waypointData.back();
	switch (data.commandID)
	{
	case _WaypointFly:
		//判断飞行高度
		if (data.z > _MaxFlyHeight_) {
			QString error = QString(data.message + tr("飞出限制高度，设定值[%1]厘米，最大高度[%2]厘米")
				.arg(data.z).arg(_MaxFlyHeight_));
			showWaypointError(error, data.blockid);
			return nullptr;
		}
#ifndef _DebugApp_
		//判断是否飞出场地范围
		if (data.x < 100 || data.y < 100 || data.x >(g_nSpaceX - 100) || data.y >(g_nSpaceY - 100)) {
			QString error = QString(data.message + tr("飞出限制区域，设定值[X:%1 Y:%2]厘米，限制区域[X:100--%3 Y:100--%4]厘米")
				.arg(data.x).arg(data.y).arg(g_nSpaceX - 100).arg(g_nSpaceY - 100));
			showWaypointError(error, data.blockid);
			return nullptr;
		}
#endif
		//判断飞行速度 最大飞行速度200厘米/秒
		if (g_waypointData.count() > 1) {
			for (int i = g_waypointData.count() - 2; i >= 0; i--) {
				NavWayPointData last = g_waypointData.at(i);
				if (_WaypointFly != last.commandID && _WaypointFlyTakeOff != last.commandID) continue;
				int d = getDistance(last.x, last.y, last.z, data.x, data.y, data.z);
				if (qAbs(d) > 0 && qAbs(data.param3) > 0) {
					float speed = qAbs(d) / data.param3;
					if (speed > 200.0) {
						showWaypointError(data.message+tr("超出最大飞行速度每秒2米"), data.blockid);
						return nullptr;
					}
				}
				break;
			}
		}
		g_nTimeTotal += data.param3;
		break;
	case _WaypointSpeed:
		//飞行速度范围
		if (data.param1 > 200 || data.param1 < 10) {
			showWaypointError(QString(tr("飞行速度设定超出范围，设定值:[%1]，最小最大值[%2-%3]")
				.arg(data.param1).arg(10).arg(200)), data.blockid);
			return nullptr;
		}
		break;
	case _WaypointRevolve:
		//旋转角度
		if (data.param1 > 360 || data.param1 < -360) {
			showWaypointError(tr("旋转角度设定超出范围"), data.blockid);
			return nullptr;
		}
		g_nTimeTotal += data.param3;
		break;
	case _WaypointHover:
		//悬停时间
		if (data.param1 > 200 || data.param1 < 0.1) {
			showWaypointError(QString(tr("悬停时间设定超出范围，设定值:[%1]秒，最小最大值[%2-%3]秒")
				.arg(data.param1).arg(0).arg(200)), data.blockid);
			return nullptr;
		}
		g_nTimeTotal += data.param3;
		break;
	case _WaypointLedStatus:
		//LED灯模式
		if (data.param1 > 10 || data.param1 < 6) {
			showWaypointError(tr("LED模式选择超出范围"), data.blockid);
			return nullptr;
		}
		break;
	case _WaypointStart:
		//只有第一步才能是起飞位置
		if (g_waypointData.count() != 2) {
			//第一条为初始位置，第二条为起飞
			showWaypointError(tr("起飞位置放置错误"), data.blockid);
			return nullptr;
		}
		g_nTimeTotal += data.param3;
		break;
	case _WaypointTime:
		break;
	case _WaypointFlyLand:
		g_bLand = true;
		break;
	case _WaypointFlyTakeOff:
		break;
	case _WaypointLedColor:
		break;
	default:
		showWaypointError(tr("内部错误舞步中存在无法使用积木块"), data.blockid);
		return nullptr;
		break;
	}
	if (g_waypointData.count() >= 2) {
		NavWayPointData second = g_waypointData.at(1);
		if (false == second.bTakeoff) {
			showWaypointError("起飞动作缺少或顺序错误", data.blockid);
			return nullptr;
		}
	}
	return Py_BuildValue("i", 0);
}

void QZAPI::showWaypointError(QString error, QString id)
{
	QString text = QString("%1 %2").arg(g_deviceName).arg(error);
	_ShowErrorMessage(text);
	m_bBlocklyError = true;
	blockFlicker(id);
}

void QZAPI::blockFlicker(QString id)
{
	//发送积木块ID到blockly中，使其在网页中闪烁显示
	if (id.isEmpty()) return;
	emit sigBlockFlicker(id);
}

QZAPI QZAPI::m_qzaip;
////Python to c++ data start////
//python接口格式，注册c++函数到python中，便于通过python调用c++函数
PyMethodDef xWrapMethods[] = {	
	//手动编写python使用接口
	{"FlyAddMarkPoint", QZAPI::FlyAddMarkPoint,		METH_VARARGS, "FlyAddMarkPoint"},
	{"FlySetSpeed",		QZAPI::FlySetSpeed,			METH_VARARGS, "FlySetSpeed"},
	{"FlySetLedMode",	QZAPI::FlySetLedMode,		METH_VARARGS, "FlySetLedMode"},
	{"FlySetLedColor",	QZAPI::FlySetLedColor,		METH_VARARGS, "FlySetLedColor"},
	{"FlyHover",		QZAPI::FlyHover,			METH_VARARGS, "FlyHover"},
	{"FlyTakeoff",		QZAPI::FlyTakeoff,			METH_VARARGS, "FlyTakeoff"},
	{"FlyTakeoffDelay",	QZAPI::FlyTakeoffDelay,		METH_VARARGS, "FlyTakeoffDelay"},
	{"FlyLand",			QZAPI::FlyLand,				METH_VARARGS, "FlyLand"},
	{"FlyTimeGroup",	QZAPI::FlyTimeGroup,		METH_VARARGS, "FlyTimeGroup"},
	{"FlyRevolve",		QZAPI::FlyRevolve,			METH_VARARGS, "FlyRevolve"},
	{"FlyTo",			QZAPI::FlyTo,				METH_VARARGS, "FlyTo"},
	{"FlyMove",			QZAPI::FlyMove,				METH_VARARGS, "FlyMove"},
	{"FlyToMarkPoint",	QZAPI::FlyToMarkPoint,		METH_VARARGS, "FlyToMarkPoint"},
	//blockly积木块转换python接口
	{"Fly_AddMarkPoint",QZAPI::FlyAddMarkPoint,		METH_VARARGS, "FlyAddMarkPoint"},
	{"Fly_SetSpeed",	QZAPI::FlySetSpeed,			METH_VARARGS, "FlySetSpeed"},
	{"Fly_SetLedMode",	QZAPI::FlySetLedMode,		METH_VARARGS, "FlySetLedMode"},
	{"Fly_SetLedColor",	QZAPI::FlySetLedColor,		METH_VARARGS, "FlySetLedColor"},
	{"Fly_Hover",		QZAPI::FlyHover,			METH_VARARGS, "FlyHover"},
	{"Fly_Takeoff",		QZAPI::FlyTakeoff,			METH_VARARGS, "FlyTakeoff"},
	{"Fly_TakeoffDelay",QZAPI::FlyTakeoffDelay,		METH_VARARGS, "FlyTakeoffDelay"},
	{"Fly_Land",		QZAPI::FlyLand,				METH_VARARGS, "FlyLand"},
	{"Fly_TimeGroup",	QZAPI::FlyTimeGroup,		METH_VARARGS, "FlyTimeGroup"},
	{"Fly_Revolve",		QZAPI::FlyRevolve,			METH_VARARGS, "FlyRevolve"},
	{"Fly_To",			QZAPI::FlyTo,				METH_VARARGS, "FlyTo"},
	{"Fly_Move",		QZAPI::FlyMove,				METH_VARARGS, "FlyMove"},
	{"Fly_ToMarkPoint",	QZAPI::FlyToMarkPoint,		METH_VARARGS, "FlyToMarkPoint"},
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

PyObject* QZAPI::FlyAddMarkPoint(PyObject* self, PyObject* args)
{
	char* name = NULL;
	int x, y, z;
	x = y = z = 0;
	char* id = NULL;
	if (!PyArg_ParseTuple(args, "s|i|i|i|s", &name, &x, &y, &z, &id)) {
		QZAPI::Instance()->showWaypointError(tr("标定点参数值错误"));
		return nullptr;
	}
	QString qstrId(id);
	if (NULL == name) {
		QZAPI::Instance()->showWaypointError(tr("标定点名称值错误"), qstrId);
		return nullptr;
	}
	//判断是否飞出场地范围
	if (x < 100 || y < 100 || x >(g_nSpaceX - 100) || y >(g_nSpaceY - 100)) {
		QZAPI::Instance()->showWaypointError(tr("标定点超出场地范围"), qstrId);
		return nullptr;
	}
	QString qstrName(name);
	qDebug() << "添加标定点" << qstrName << x << y << z;
	if (g_mapMarkPoint.contains(qstrName)) {
		QZAPI::Instance()->showWaypointError(tr("存在相同名称标定点 ")+qstrName, qstrId);
		return nullptr;
	}
	g_mapMarkPoint.insert(qstrName, _MarkPoint(x,y,z));
	return Py_BuildValue("i", 0);
}

PyObject* QZAPI::FlySetSpeed(PyObject* self, PyObject* args)
{
	//设置飞行速度废弃
	return Py_BuildValue("i", 0);
	int n = 0;
	char* id = NULL;
	if (!PyArg_ParseTuple(args, "i|s", &n, &id)) {
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
	data.blockid = QString(id);
	data.commandID = _WaypointSpeed;
	g_waypointData.append(data);
	return QZAPI::Instance()->examineWaypoint();
}

PyObject* QZAPI::FlySetLedMode(PyObject* self, PyObject* args)
{
	int n = 0;
	char* id = NULL;
	if (!PyArg_ParseTuple(args, "i|s", &n, &id)) {
		QZAPI::Instance()->showWaypointError(tr("LED灯参数值错误"));
		return nullptr;
	}
	qDebug() << "设置LED灯模式" << n;
	NavWayPointData last = g_waypointData.back();
	NavWayPointData data;
	data.blockid = QString(id);
	data.x = last.x;
	data.y = last.y;
	data.z = last.z;
	data.param1 = n;
	data.commandID = _WaypointLedStatus;
	g_waypointData.append(data);
	return QZAPI::Instance()->examineWaypoint();
}

PyObject* QZAPI::FlySetLedColor(PyObject* self, PyObject* args)
{
	char* color = NULL;
	char* id = NULL;
	if (!PyArg_ParseTuple(args, "s|s", &color, &id)) {
		QZAPI::Instance()->showWaypointError(tr("LED颜色值设置有错误"));
		return nullptr;
	}
	QString qstrId(id);
	if (NULL == color) {
		QZAPI::Instance()->showWaypointError(tr("LED颜色值无法使用"), qstrId);
		return nullptr;
	}
	QColor qc(color);
	NavWayPointData data;
	data.blockid = qstrId;
	data.param1 = qc.red();
	data.param2 = qc.green();
	data.param3 = qc.blue();
	qDebug() << "设置LED灯颜色" << color << data.param1 << data.param2 << data.param3;
	NavWayPointData last = g_waypointData.back();
	data.x = last.x;
	data.y = last.y;
	data.z = last.z;
	data.commandID = _WaypointLedColor;
	g_waypointData.append(data);
	return QZAPI::Instance()->examineWaypoint();
}

PyObject* QZAPI::FlyHover(PyObject* self, PyObject* args)
{
	int n = 0;
	char* id = NULL;
	if (!PyArg_ParseTuple(args, "i|s", &n, &id)) {
		QZAPI::Instance()->showWaypointError(tr("悬停参数值错误"));
		return nullptr;
	}
	qDebug() << "悬停时间" << n << "秒";
	NavWayPointData last = g_waypointData.back();
	NavWayPointData data;
	data.blockid = QString(id);
	data.x = last.x;
	data.y = last.y;
	data.z = last.z;
	data.param1 = n;
	data.param3 = data.param1;
	data.commandID = _WaypointHover;
	//悬停使用飞行航点
	data.commandID = _WaypointFly;
	g_waypointData.append(data);
	return QZAPI::Instance()->examineWaypoint();
}

PyObject* QZAPI::FlyTakeoff(PyObject* self, PyObject* args)
{
	int n = 0;
	char* id = NULL;
	NavWayPointData data;
	int millisecond = 0;
	if (!PyArg_ParseTuple(args, "i|i|s", &n, &millisecond, &id)) {
		QZAPI::Instance()->showWaypointError(tr("起飞参数值错误"));
		return nullptr;
	}
	data.blockid = QString(id);
	data.param3 = millisecond;
	qDebug() << "起飞" << n << millisecond;
	NavWayPointData lastWaypoint = g_waypointData.back();
	if (_WaypointStart != lastWaypoint.commandID) {
		QZAPI::Instance()->showWaypointError(tr("起飞必须是第一步"), data.blockid);
		return nullptr;
	}
	if (millisecond < 5) {
		QZAPI::Instance()->showWaypointError(tr("起飞用时最少5秒钟"));
		return nullptr;
	}
	data.x = lastWaypoint.x;
	data.y = lastWaypoint.y;
	data.z = n;
	data.bTakeoff = true;
	data.commandID = _WaypointFlyTakeOff;
	g_waypointData.append(data);
	return QZAPI::Instance()->examineWaypoint();
}

PyObject* QZAPI::FlyTakeoffDelay(PyObject* self, PyObject* args)
{
	int n = 0;
	char* id = NULL;
	NavWayPointData data;
	int millisecond = 0;
	int delay = 0;
	if (!PyArg_ParseTuple(args, "i|i|i|s", &delay, &n, &millisecond, &id)) {
		QZAPI::Instance()->showWaypointError(tr("起飞参数值错误"));
		return nullptr;
	}
	data.blockid = QString(id);
	data.param3 = millisecond;
	qDebug() << "延时起飞" << delay << n << millisecond;
	NavWayPointData lastWaypoint = g_waypointData.back();
	if (_WaypointStart != lastWaypoint.commandID) {
		QZAPI::Instance()->showWaypointError(tr("起飞必须是第一步"), data.blockid);
		return nullptr;
	}
	g_waypointData[0].param1 = delay;
	g_waypointData[0].param3 = delay;
	data.x = lastWaypoint.x;
	data.y = lastWaypoint.y;
	data.z = n;
	data.bTakeoff = true;
	data.commandID = _WaypointFlyTakeOff;
	g_waypointData.append(data);
	return QZAPI::Instance()->examineWaypoint();
}

PyObject* QZAPI::FlyLand(PyObject* self, PyObject* args)
{
	qDebug() << "降落";
	char* id = NULL;
	if (g_bBlockly) {
		PyArg_ParseTuple(args, "s", &id);
	}
	NavWayPointData lastWaypoint = g_waypointData.back();
	NavWayPointData data;
	data.blockid = QString(id);
	data.commandID = _WaypointFlyLand;
	data.param3 = 0.5;
	data.x = lastWaypoint.x;
	data.y = lastWaypoint.y;
	data.z = 0;
	g_waypointData.append(data);
	return QZAPI::Instance()->examineWaypoint();
}

PyObject* QZAPI::FlyTimeGroup(PyObject* self, PyObject* args)
{
	self;
	char* name = NULL;
	int m, s;
	m = s = 0;
	char* id = NULL;
	if (!PyArg_ParseTuple(args, "s|i|i|s", &name, &m, &s, &id)) {
		QZAPI::Instance()->showWaypointError(tr("动作组参数错误"));
		return nullptr;
	}
	QString qstrId(id);
	qDebug() << "动作组" << name << m << s;
	if (s >= 60 || m < 0) {
		QZAPI::Instance()->showWaypointError(tr("动作组时间范围设定错误"), qstrId);
		return nullptr;
	}
	if (NULL == name) {
		QZAPI::Instance()->showWaypointError(tr("动作组名称设置错误"), qstrId);
		return nullptr;
	}
	QString qstrName(name);
	if (qstrName.isEmpty()) {
		QZAPI::Instance()->showWaypointError(tr("动作组名称必须输入"), qstrId);
		return nullptr;
	}
	if (g_mapTimeGroup.contains(qstrName)) {
		QZAPI::Instance()->showWaypointError(QString("动作%1 开始时间第%2分%3秒，名称重复").arg(qstrName).arg(m).arg(s), qstrId);
		return nullptr;
	}
	g_mapTimeGroup.insert(qstrName, g_nTimeTotal);
	NavWayPointData last = g_waypointData.back();
	NavWayPointData data;
	data.blockid = qstrId;
	data.groupname = qstrName;
	data.x = last.x;
	data.y = last.y;
	data.z = last.z;
	data.param1 = m * 60 + s;
	data.commandID = _WaypointTime;
	g_waypointData.append(data);
	return QZAPI::Instance()->examineWaypoint();
}

PyObject* QZAPI::FlyRevolve(PyObject* self, PyObject* args)
{
	//暂时去掉角度旋转
	return Py_BuildValue("i", 0);
	float angle = 0.0;
	NavWayPointData data;
	int millisecond = 0;
	char* id = NULL;
	if (!PyArg_ParseTuple(args, "f|i|s", &angle, &millisecond, &id)) {
		QZAPI::Instance()->showWaypointError(tr("旋转参数值错误"));
		return nullptr;
	}
	data.blockid = QString(id);
	data.param3 = millisecond;
	qDebug() << "无人机旋转" << angle;
	NavWayPointData last = g_waypointData.back();
	data.x = last.x;
	data.y = last.y;
	data.z = last.z;
	//data.param1 = angle;
	//data.commandID = _WaypointRevolve;
	////旋转使用飞行航点加旋转信息
	data.param4 = angle;
	data.commandID = _WaypointFly;
	g_waypointData.append(data);
	return QZAPI::Instance()->examineWaypoint();
}

PyObject* QZAPI::FlyTo(PyObject* self, PyObject* args)
{
	int x, y, z;
	x = y = z = 0;
	NavWayPointData data;
	int millisecond = 0;
	char* id = NULL;
	if (!PyArg_ParseTuple(args, "i|i|i|i|s", &x, &y, &z, &millisecond, &id)) {
		QZAPI::Instance()->showWaypointError(tr("飞行到参数值错误"));
		return nullptr;
	}
	data.blockid = QString(id);
	data.param3 = millisecond;
	qDebug() << "飞行到绝对位置" << x << y << z;
	data.x = x;
	data.y = y;
	data.z = z;
	g_waypointData.append(data);
	return QZAPI::Instance()->examineWaypoint();
}

PyObject* QZAPI::FlyMove(PyObject* self, PyObject* args)
{
	int direction, n;
	direction = n = 0;
	NavWayPointData data;
	int millisecond = 0;
	char* id = NULL;
	if (!PyArg_ParseTuple(args, "i|i|i|s", &direction, &n, &millisecond, &id)) {
		QZAPI::Instance()->showWaypointError(tr("向固定方向飞行参数值错误"));
		return nullptr;
	}
	data.blockid = QString(id);
	data.param3 = millisecond;
	qDebug() << "相对移动" << direction << n;
	if (direction > 6) {
		QZAPI::Instance()->showWaypointError(tr("飞行方向参数错误"), data.blockid);
		return nullptr;
	}
	NavWayPointData lastWaypoint = g_waypointData.back();
	data.x = lastWaypoint.x;
	data.y = lastWaypoint.y;
	data.z = lastWaypoint.z;
	//["前", "1"],["后", "2"],["右", "3"],["左", "4"],["上", "5"],["下", "6"],
	int angle = 0;
	for (int i = 0; i < g_waypointData.count(); i++) {
		NavWayPointData way = g_waypointData.at(i);
		if(_WaypointRevolve == way.commandID) angle += way.param1;
		if (_WaypointFly == way.commandID) angle += way.param4;
	}
	if (0 == angle) {
		switch (direction){
		case 1: data.x += n; data.message = tr("向前飞行"); break;
		case 2: data.x -= n; data.message = tr("向后飞行"); break;
		case 3: data.y += n; data.message = tr("向右飞行"); break;
		case 4: data.y -= n; data.message = tr("向左飞行"); break;
		case 5: data.z += n; data.message = tr("向上飞行"); break;
		case 6: data.z -= n; data.message = tr("向下飞行"); break;
		}
	}
	else {
		//根据旋转角度计算飞行方向
		angle = angle % 360;
		double radian = angle * M_PI / 180;
		qreal temp = qSin(radian);
		switch (direction) {
		case 1: 
			data.x += qCos(radian) * n;
			data.y += qSin(radian) * n;
			data.message = tr("向前飞行");
			break;
		case 2: 
			data.x -= qCos(radian) * n;
			data.y -= qSin(radian) * n;
			data.message = tr("向后飞行");
			break;
		case 3: 
			data.x -= qSin(radian) * n;
			data.y -= qCos(radian) * n;
			data.message = tr("向右飞行");
			break;
		case 4: 
			data.x += qSin(radian) * n;
			data.y += qCos(radian) * n;
			data.message = tr("向左飞行");
			break;
		case 5: data.z += n; data.message = tr("向上飞行"); break;
		case 6: data.z -= n; data.message = tr("向下飞行"); break;
		}
	}
	data.message.append(QString("%1厘米用时%2秒动作").arg(n).arg(data.param3));
	g_waypointData.append(data);
	return QZAPI::Instance()->examineWaypoint();
}

PyObject* QZAPI::FlyToMarkPoint(PyObject* self, PyObject* args)
{
	char* name = NULL;
	NavWayPointData data;
	char* id = NULL;
	int millisecond = 0;
	if (!PyArg_ParseTuple(args, "s|i|s", &name, &millisecond, &id)) {
		QZAPI::Instance()->showWaypointError(tr("飞到标定点参数值错误"));
		return nullptr;
	}
	data.param3 = millisecond;
	data.blockid = QString(id);
	if (NULL == name) {
		QZAPI::Instance()->showWaypointError(tr("飞到标定点名称错误"), data.blockid);
		return nullptr;
	}
	QString qstrName(name);
	if (qstrName.isEmpty()) {
		QZAPI::Instance()->showWaypointError(tr("飞到标定点名称必须输入"), data.blockid);
		return nullptr;
	}
	if (false == g_mapMarkPoint.contains(qstrName)) {
		QZAPI::Instance()->showWaypointError(qstrName + tr("标定点未添加"), data.blockid);
		return nullptr;
	}
	_MarkPoint point = g_mapMarkPoint[qstrName];
	qDebug() << "飞行到标定点" << qstrName << point.x << point.y << point.z;
	data.x = point.x;
	data.y = point.y;
	data.z = point.z;
	data.commandID = _WaypointFly;
	g_waypointData.append(data);
	return QZAPI::Instance()->examineWaypoint();
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
	m_bBlocklyError = false;
}

ThreadPython::~ThreadPython()
{
	if (isRunning()) {
		quit();
		wait();
	}
}

void ThreadPython::initParam(unsigned int nSpaceX, unsigned int nSapaceY, QString name
	, unsigned int nStartX, unsigned int nStartY, bool blockly)
{
	//因为是全局变量，使用前清空内容
	g_waypointData.clear();
	g_mapMarkPoint.clear();
	g_mapTimeGroup.clear();
	g_nTimeTotal = 0;
	g_bLand = false;
	g_nSpaceX = nSpaceX;
	g_nSpaceY = nSapaceY;
	g_deviceName = name;
	g_bBlockly = blockly;
	//舞步前添加起始位置
	NavWayPointData startLocation;
	startLocation.x = nStartX;
	startLocation.y = nStartY;
	startLocation.z = 0;
	startLocation.commandID = _WaypointStart;
	g_waypointData.append(startLocation);
}

bool ThreadPython::compilePythonCode(QByteArray arrCode)
{
	if(arrCode.isEmpty()) return false;
	m_pythonState = PythonRunNone;
	m_bBlocklyError = false;
	//python代码保存至文件后执行
	QString path = QApplication::applicationDirPath() + _PyRunDir_;
	//删除python运行目录，防止缓存影响
	deleteDir(path);
	QDir dir(path);
	//新建python运行目录
	dir.mkdir(path);
	QString qstrRunFile = path + _PyRunFilePath_;
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


bool ThreadPython::isBlocklyError()
{
	return m_bBlocklyError;
}

QMap<QString, unsigned int> ThreadPython::getTimeGroup()
{
	return g_mapTimeGroup;
}

unsigned int ThreadPython::getFlyTotalTime()
{
	return g_nTimeTotal;
}

QString ThreadPython::getErrorString(int state)
{
	QString error = "未知错误";
	switch (state)
	{
	case PythonRunNone: error = "编程解析失败"; break;
	case PythonError: error = "编程解析失败"; break;
	case PythonFileError: error = "编程文件不存在"; break;
	case PythonTimeout: error = "编程文件不存在"; break;
	case PythonCodeError: error = "编程代码错误"; break;
	case PythonWaypointError: error = "舞步转换失败"; break;
	case PythonWaypointNull: error = "没有编写舞步"; break;
	default:
		break;
	}
	return error;
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

void ThreadPython::print_python_error()
{
	// 获取异常信息
	PyObject* type, * value, * traceback;
	PyErr_Fetch(&type, &value, &traceback); // 获取当前异常信息
	PyErr_NormalizeException(&type, &value, &traceback); // 规范化异常
	if (type) {
		PyObject* type_str = PyObject_Str(type);
		QString error = PyUnicode_AsUTF8(type_str);
		Py_XDECREF(type_str);
	}

	if (value) {
		PyObject* value_str = PyObject_Str(value);
		_ShowErrorMessage(PyUnicode_AsUTF8(value_str));
		Py_XDECREF(value_str);
	}

	if (traceback) {
		PyObject* traceback_module = PyImport_ImportModule("traceback");
		if (traceback_module) {
			PyObject* format_tb_func = PyObject_GetAttrString(traceback_module, "format_tb");
			if (format_tb_func && PyCallable_Check(format_tb_func)) {
				PyObject* tb_list = PyObject_CallFunctionObjArgs(format_tb_func, traceback, NULL);
				if (tb_list) {
					PyObject* tb_str = PyUnicode_FromString("");
					for (Py_ssize_t i = 0; i < PyList_Size(tb_list); ++i) {
						PyObject* item = PyList_GetItem(tb_list, i);
						tb_str = PyUnicode_Concat(tb_str, item);
					}
					QString error = PyUnicode_AsUTF8(tb_str);
					//_ShowErrorMessage(error);
					int index = error.indexOf(", line ");
					if (index > 0) {
						QString text = error.mid(index + QString(", line ").length());
						_ShowErrorMessage(QString("错误代码所在行：") + text.replace("\n", ""));
					}
					Py_DECREF(tb_str);
				}
				Py_XDECREF(tb_list);
			}
			Py_XDECREF(format_tb_func);
		}
		Py_XDECREF(traceback_module);
	}
	//_ShowErrorMessage(errorText);
	// 清理异常
	Py_XDECREF(type);
	Py_XDECREF(value);
	Py_XDECREF(traceback);
}

void ThreadPython::run()
{
	if (!QFile::exists(m_qstrFilePath)) return;
	qDebug() << "python文件开始执行" << m_qstrFilePath;
	int n = PyRun_SimpleString("import sys");
	QString qstrPythonPath = QString("sys.path.append('%1')").arg(QApplication::applicationDirPath()+_PyRunDir_);
	//设置python执行文件路径
	//PyRun_SimpleString(qstrPythonApiPath.toStdString().c_str());
	PyRun_SimpleString(qstrPythonPath.toStdString().c_str());
	//执行python文件,生成航点列表
	PyObject* pPyFile = PyImport_ImportModule(_PyFileName_);
	if (!pPyFile) {
		print_python_error();
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
