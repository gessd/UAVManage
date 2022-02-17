#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <QAudioDecoder>
#include <QAudioBuffer>
#include <QList>

using bufferList = QList<QAudioBuffer>;

class AudioDecoder : public QAudioDecoder
{
    Q_OBJECT
private:
    bufferList m_bufferList;            //解码后的数据列表
    bool m_isFinished = false;          //解码是否完成
public:
    AudioDecoder(QObject *parent = 0);
    ~AudioDecoder();
public:
    void analysisAudioFile(const QString& fileName);    //解码音频文件
    const bufferList&  getBufferList()const;            //获取解码后的文件

private:
    void CreateSignalConnectSlot();
};

#endif // AUDIODECODER_H
