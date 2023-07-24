#include "YmodemFileTransmit.h"
#include <QFileInfo>

#define READ_TIME_OUT   (10)
#define WRITE_TIME_OUT  (100)

YmodemFileTransmit::YmodemFileTransmit(QObject *parent) :
    QObject(parent),
    file(new QFile),
    readTimer(new QTimer),
    writeTimer(new QTimer)

{
    serialPort = nullptr;

    setTimeDivide(499);
    setTimeMax(5);
    setErrorMax(999);

    connect(readTimer, SIGNAL(timeout()), this, SLOT(readTimeOut()));
    connect(writeTimer, SIGNAL(timeout()), this, SLOT(writeTimeOut()));
}

YmodemFileTransmit::~YmodemFileTransmit()
{
    delete file;
    delete readTimer;
    delete writeTimer;
}

bool YmodemFileTransmit::startTransmit(QSerialPort* serial, QString filename, QString portname, qint32 baudrate)
{
	file->setFileName(filename);
	serialPort = serial;
    progress = 0;
    status   = StatusEstablish;
    readTimer->start(READ_TIME_OUT);
    return true;
}

void YmodemFileTransmit::stopTransmit()
{
    file->close();
    abort();
    status = StatusAbort;
    writeTimer->start(WRITE_TIME_OUT);
}

int YmodemFileTransmit::getTransmitProgress()
{
    return progress;
}

Ymodem::Status YmodemFileTransmit::getTransmitStatus()
{
    return status;
}

void YmodemFileTransmit::readTimeOut()
{
    readTimer->stop();

    transmit();

    if((status == StatusEstablish) || (status == StatusTransmit))
    {
        readTimer->start(READ_TIME_OUT);
    }
}

void YmodemFileTransmit::writeTimeOut()
{
    writeTimer->stop();
    transmitStatus(status);
}

Ymodem::Code YmodemFileTransmit::callback(Status status, uint8_t *buff, uint32_t *len)
{
    switch(status)
    {
        case StatusEstablish:
        {
            if(file->open(QFile::ReadOnly) == true)
            {
                QFileInfo fileInfo(*file);

                fileSize  = fileInfo.size();
                fileCount = 0;

                strcpy((char *)buff, fileInfo.fileName().toLocal8Bit().data());
                strcpy((char *)buff + fileInfo.fileName().toLocal8Bit().size() + 1, QByteArray::number(fileInfo.size()).data());

                *len = YMODEM_PACKET_SIZE;

                YmodemFileTransmit::status = StatusEstablish;

                transmitStatus(StatusEstablish);

                return CodeAck;
            }
            else
            {
                YmodemFileTransmit::status = StatusError;

                writeTimer->start(WRITE_TIME_OUT);

                return CodeCan;
            }
        }

        case StatusTransmit:
        {
            if(fileSize != fileCount)
            {
                if((fileSize - fileCount) > YMODEM_PACKET_SIZE)
                {
                    fileCount += file->read((char *)buff, YMODEM_PACKET_1K_SIZE);

                    *len = YMODEM_PACKET_1K_SIZE;
                }
                else
                {
                    fileCount += file->read((char *)buff, YMODEM_PACKET_SIZE);

                    *len = YMODEM_PACKET_SIZE;
                }

                progress = (int)(fileCount * 100 / fileSize);

                YmodemFileTransmit::status = StatusTransmit;

                transmitProgress(progress);
                transmitStatus(StatusTransmit);

                return CodeAck;
            }
            else
            {
                YmodemFileTransmit::status = StatusTransmit;

                transmitStatus(StatusTransmit);

                return CodeEot;
            }
        }

        case StatusFinish:
        {
            file->close();

            YmodemFileTransmit::status = StatusFinish;

            writeTimer->start(WRITE_TIME_OUT);

            return CodeAck;
        }

        case StatusAbort:
        {
            file->close();

            YmodemFileTransmit::status = StatusAbort;

            writeTimer->start(WRITE_TIME_OUT);

            return CodeCan;
        }

        case StatusTimeout:
        {
            YmodemFileTransmit::status = StatusTimeout;

            writeTimer->start(WRITE_TIME_OUT);

            return CodeCan;
        }

        default:
        {
            file->close();

            YmodemFileTransmit::status = StatusError;

            writeTimer->start(WRITE_TIME_OUT);

            return CodeCan;
        }
    }
}

uint32_t YmodemFileTransmit::read(uint8_t *buff, uint32_t len)
{
    qint64 res = 0;
    if (serialPort) {
        res = serialPort->read((char*)buff, len);
    }
    return res;
}

uint32_t YmodemFileTransmit::write(uint8_t *buff, uint32_t len)
{
    qint64 res = 0;
    if(serialPort) 
        res = serialPort->write((char*)buff, len);
    return res;
}
