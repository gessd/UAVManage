#include "paramreadwrite.h"
#include <QApplication>
#include <QDebug>

ParamReadWrite::ParamReadWrite(QObject *parent) : QObject(parent)
{

}

QVariant ParamReadWrite::readParam(QString qstrKey, QVariant defaultValue, QString qstrGroup, QString file)
{
    QString qstrPath = file;// QApplication::applicationDirPath() + PARAMFILE;
    QSettings settings(qstrPath, QSettings::IniFormat);
    settings.setIniCodec("UTF8");
    settings.beginGroup(qstrGroup);
    if(!settings.contains(qstrKey)){
        writeParam(qstrKey, defaultValue, qstrGroup);
    }
    QVariant value = settings.value(qstrKey, defaultValue);
    settings.endGroup();
    return value;
}

void ParamReadWrite::writeParam(QString qstrKey, QVariant value, QString qstrGroup, QString file)
{
    QString qstrPath = file;// QApplication::applicationDirPath() + PARAMFILE;
    QSettings settings(qstrPath, QSettings::IniFormat);
    settings.setIniCodec("UTF8");
    settings.beginGroup(qstrGroup);
    settings.setValue(qstrKey, value);
    settings.endGroup();
    return;
}
