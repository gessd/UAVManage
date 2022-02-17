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
	void updateLoadMusic(QString filePath);
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

	QString  durationTime;			//总长度
	QString  positionTime;			//当前播放到位置
signals:
	void startPlay();
	void isPlaying(bool);			//是否在播放中信号
	void playeState(qint8);			//播放状态信号 1:开始 2:暂停 3:结束	
	void sigUpdateMusic(QString path);
private:
	void signalConnectSlot();
private slots:
	//自定义槽函数
	void onDecoderFinished();		//解析器解析完毕槽函数
	void onStateChanged(QMediaPlayer::State state);

	void onDurationChanged(qint64 duration);
	void onPositionChanged(qint64 position);
	void on_m_pBtnPlay_clicked();	//播放按钮
	void on_m_pBtnPrev_clicked();	//上一首
	void on_m_pBtnNext_clicked();	//上一首
	void on_m_pBtnLoad_clicked();	//添加文件
	void on_m_pHslider_sliderPressed();								//拖动滑块后进行音频播放进度调控
	void on_m_pHslider_sliderReleased();							//拖动滑块后进行音频播放进度调控
	void on_m_pPlayList_currentRowChanged(int currentRow);			//单击播放列表
	void on_m_pPlayList_doubleClicked(const QModelIndex &index);	//双击播放列表
	
};
#endif 