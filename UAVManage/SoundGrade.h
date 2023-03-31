#ifndef SOUNDGRADE_H
#define SOUNDGRADE_H
#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QAudioProbe>
#include <QAudioFormat>
#include "AudioDecoder.h"
#include "ui_SoundGrade.h"

class SoundGrade : public QWidget
{
    Q_OBJECT
public:
	SoundGrade(QWidget *parent = 0);
    ~SoundGrade();
	/**
	 * @brief 清空播放区域数据
	 */
	void clearSound();
	/**
	 * @brief 更新播放音乐路径
	 */
	void updateLoadMusic(QString filePath);
	/*
	* @brief 音乐波形图
	*/
	QPixmap getMusicPixmap();
	/*
	* @brief 音乐文件路径
	*/
	QString getCurrentMusic();
public slots:
	void startPlayMusic();
    void stopPlayMusic();
private:
	Ui_SoundGrade m_ui;
    //QTimer timer;					//定时器
    QVector<float> m_vecSpecData;
    QString m_strCurFile;			//当前播放文件
    bool m_bPlaying = false;		//是否正在播放
	bool m_sliderPlaying = false;	//调整滑块前播放状态

	QMediaPlayer    *player;		//播放器
	AudioDecoder    *m_decoder;		//解码器
	QAudioProbe     *m_probe;		//探测器
	QMediaPlaylist  *playlist;		//播放列表

	qint64 m_nDuration;		//音乐时长 毫秒
	QLabel* m_pLabelPosition;
signals:
	void startPlay();
	void isPlaying(bool);			//是否在播放中信号
	void playeState(qint8);			//播放状态信号 1:开始 2:暂停 3:结束	
	void sigUpdateMusic(QString path);
	void sigMsuicTime(unsigned int second);
	void updateMusicWaveFinished();
private:
	void signalConnectSlot();
private slots:
	//自定义槽函数
	void onDecoderFinished();		//解析器解析完毕槽函数
	void onStateChanged(QMediaPlayer::State state);

	void onDurationChanged(qint64 duration);
	void onPositionChanged(qint64 position);
	void on_m_pBtnPlay_clicked();	//播放按钮
	//void on_m_pBtnPrev_clicked();	//上一首
	//void on_m_pBtnNext_clicked();	//上一首
	void on_m_pBtnLoad_clicked();	//添加文件
	void on_m_pHslider_sliderPressed();								//拖动滑块后进行音频播放进度调控
	void on_m_pHslider_sliderReleased();							//拖动滑块后进行音频播放进度调控
};
#endif 