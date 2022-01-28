#include "globalfunction.h"

QString Utility::waypointMessgeFromStatus(int status)
{
	QString qstrMessage;
	switch (status)
	{
	case DeviceMessageToimeout:
		qstrMessage = QObject::tr("超时");
		break;
	case DeviceMessageSending:
		qstrMessage = QObject::tr("进行中");
		break;
	case DeviceDataError:
		qstrMessage = QObject::tr("数据错误");
		break;
	case DeviceUnConnect:
		qstrMessage = QObject::tr("网络未连接");
		break;
	case DeviceWaiting:
		qstrMessage = QObject::tr("错误");
		break;
	case DeviceDataSucceed:
		qstrMessage = QObject::tr("成功");
		break;
	}
	return qstrMessage;
}