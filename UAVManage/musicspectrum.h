#pragma once

#include <QWidget>
#include <QAudioDecoder>
#include <QAudioBuffer>
#include <QAudioFormat>
#include <QLabel>

class MusicSpectrum  : public QWidget
{
	Q_OBJECT

public:
	MusicSpectrum(QWidget *parent);
	~MusicSpectrum();
	void addMusicFile(QString file, unsigned int time);
private slots:
	/**
 * @brief 音乐文件解码后数据
 */
	void onDecoderBufferReady();
	/**
	 * @brief 音乐文件解码出错
	 */
	void onDecoderError(QAudioDecoder::Error error);
	/**
	 * @brief 音乐文件解码完成
	 */
	void onDecoderFinished();
protected:
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void enterEvent(QEvent* event);
	virtual void leaveEvent(QEvent* event);
	virtual void paintEvent(QPaintEvent* event);
private:
	/**
	 * @brief 绘制音乐时域图
	 */
	void drawSpectrum();
	/**
	 * @brief 计算音乐文件音量
	 */
	int pcmToDB(const char* pcmdata, int size);
signals:
	void updateMusicWaveFinished();
private:
	//音频文件解码器
	QAudioDecoder m_audioDcoder;
	//音频文件解码后数据
	QList<QAudioBuffer> m_listAudioBuffer;
	//音频文件计算后的音量
	QList<int> m_listSpectrumData;
	unsigned int m_nMusicTime;
	//当前时间
	QLabel* m_pLaberTime;
};
