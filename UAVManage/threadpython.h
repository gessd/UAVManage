#pragma once

#include <QThread>
#include <Python.h>
#include "mypythondefine.h"
#include "definesetting.h"
#include "globalfunction.h"

class QZAPI : public QObject
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
	//设置LED模式
	static PyObject* Fly_LedMode(PyObject* self, PyObject* args);
	//单方向增量飞行
	static PyObject* Fly_Moveto(PyObject* self, PyObject* args);
	//单方向飞行
	static PyObject* Fly_MoveAddTo(PyObject* self, PyObject* args);

	/**
	 *  @brief 预设飞行点
	 * 	@param string 预设点名称，不能重复
	 *	@param int    x位置
	 *	@param int    y位置
	 *	@param int    z位置
	 */
	static PyObject* FlyAddMarkPoint(PyObject* self, PyObject* args);
	/**
	 * @brief 设置速度
	 * @param int 速度 cm/s
	 */
	static PyObject* FlySetSpeed(PyObject* self, PyObject* args);
	/**
	 * @brief 设置LED
	 * @param int 模式
	 */
	static PyObject* FlySetLed(PyObject* self, PyObject* args);
	/**
	 * @brief 悬停
	 * @param int 时间 s
	 */
	static PyObject* FlyHover(PyObject* self, PyObject* args);
	/**
	 * @brief 起飞
	 * @param int 高度 cm
	 */
	static PyObject* FlyTakeoff(PyObject* self, PyObject* args);
	/**
	 * @brief 降落
	 */
	static PyObject* FlyLand(PyObject* self, PyObject* args);
	/**
	 * @brief 时间范围
	 * @param int 分钟
	 * @param int 秒
	 */
	static PyObject* FlyTimeGroup(PyObject* self, PyObject* args);
	/**
	 * @brief 旋转
	 * @param int 旋转方向[0左|1右]
	 * @param float 角度
	 */
	static PyObject* FlyRevolve(PyObject* self, PyObject* args);
	/**
	 * @brief 飞行到绝对位置
	 * @param int  x位置
	 * @param int  y位置
	 * @param int  z位置
	 */
	static PyObject* FlyTo(PyObject* self, PyObject* args);
	/**
	 * @brief 增量飞行，相对移动
	 * @param int 方向[1前|2后|3右|4左|5上|6下]
	 * @param int 距离
	 */
	static PyObject* FlyMove(PyObject* self, PyObject* args);
	/**
	 * @brief 飞行到定位点
	 * @param string 名字 FlyAddMarkPoint预先设置
	 */
	static PyObject* FlyToMarkPoint(PyObject* self, PyObject* args);
	
	static QZAPI* Instance();
private:
	static QZAPI m_qzaip;
	QZAPI() {}
	~QZAPI() {}
	PyObject* examineWaypoint();
	void showWaypointError(QString error);
};

class ThreadPython : public QThread
{
	Q_OBJECT

public:
	ThreadPython(QObject *parent = NULL);
	~ThreadPython();
	/**
	 * @brief 设定无人机初始位置
	 * @param 设备名称
	 * @param X轴 厘米
	 * @param Y轴 厘米
	 */
	void initParam(unsigned int nSpaceX, unsigned int nSapaceY, QString name, unsigned int nStartX, unsigned int nStartY);
	/**
	 * @brief 通过Python代码交互得到航点数据
	 * @param python代码
	 * @return 执行结果
	 */
	bool compilePythonCode(QByteArray arrCode);
	/**
	 * @brief 获取执行Python代码执行状态
	 * @return python执行状态
	 */
	PythonRunState getLastState();
	/**
	 * @brief 得到航点数据
	 * @return 通过python代码交互得到的航点数据
	 */
	QVector<NavWayPointData> getWaypointData();
private:
	bool compilePythonFile(QString qstrFile);
	QString checkWaypoint();
private:
	void run();
private:
	QString m_qstrFilePath;
	PythonRunState m_pythonState;
};
