#pragma once
#include <QObject>

enum _DeviceStatus
{
	DeviceMessageToimeout = -404,
	DeviceMessageSending = -4,
	DeviceDataError = -3,
	DeviceUnConnect = -2,
	DeviceWaiting = -1,
	DeviceDataSucceed = 0
};

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

class Utility
{
public:
	static QString waypointMessgeFromStatus(int status);
};


