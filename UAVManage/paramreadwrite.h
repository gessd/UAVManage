#ifndef PARAMREADWRITE_H
#define PARAMREADWRITE_H

#include <QObject>
#include <QSettings>

#define PARAMFILE       "/config.ini"
#define _Root_          "App"
#define _Path_          "path"

class ParamReadWrite : public QObject
{
    Q_OBJECT
public:
    explicit ParamReadWrite(QObject *parent = nullptr);
    static QVariant readParam(QString qstrKey, QVariant defaultValue="", QString qstrGroup = _Root_);
    static void writeParam(QString qstrKey, QVariant value, QString qstrGroup = _Root_);
signals:

public slots:
};

#endif // PARAMREADWRITE_H
