#include <QMessageBox>
#include <QFile>
#include "AudioDecoder.h"
AudioDecoder::AudioDecoder(QObject *parent):QAudioDecoder(parent)
{
    CreateSignalConnectSlot();
}

AudioDecoder::~AudioDecoder()
{

}

void AudioDecoder::CreateSignalConnectSlot()
{//创建信号槽连接
    connect(this,&QAudioDecoder::bufferReady,[&](){
        if(this->bufferAvailable())
        {
            QAudioBuffer aBuffer = this->read();
            if(aBuffer.isValid())
                m_bufferList.push_back(aBuffer);
        }
    });
    connect(this,&QAudioDecoder::finished,[&](){
        m_isFinished = true;
    });
}

void AudioDecoder::analysisAudioFile(const QString& fileName)
{//解析音频文件
    if(!QFile::exists(fileName))
        return;
	if (state() != QAudioDecoder::StoppedState)
		stop();
	m_bufferList.clear();
    setSourceFilename(fileName);
	m_isFinished = false;
	start();
}

const bufferList &AudioDecoder::getBufferList() const
{

        return m_bufferList;
}
