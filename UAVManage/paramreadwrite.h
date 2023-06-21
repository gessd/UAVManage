#ifndef PARAMREADWRITE_H
#define PARAMREADWRITE_H

#include <QObject>
#include <QSettings>
#include <QApplication>

#define PARAMFILE       "/config.ini"
#define _Root_          "App"
#define _Path_          "path"
#define _Update_        "update"
#define _Firmware_      "Firmware"

class ParamReadWrite : public QObject
{
    Q_OBJECT
public:
    explicit ParamReadWrite(QObject *parent = nullptr);
    static QVariant readParam(QString qstrKey, QVariant defaultValue="", QString qstrGroup = _Root_, QString file = QApplication::applicationDirPath() + PARAMFILE);
    static void writeParam(QString qstrKey, QVariant value, QString qstrGroup = _Root_, QString file = QApplication::applicationDirPath() + PARAMFILE);
signals:

public slots:
};

#endif // PARAMREADWRITE_H
