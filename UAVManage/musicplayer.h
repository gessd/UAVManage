#pragma once

#include <QWidget>
#include "ui_musicplayer.h"
#include <QAudioDecoder>
#include <QAudioBuffer>
#include <QMediaPlayer>
#include <QAudioFormat>

class MusicPlayer : public QWidget
{
	Q_OBJECT

public:
	MusicPlayer(QWidget *parent = nullptr);
	~MusicPlayer();
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
	void startPlayMusic();
	void stopPlayMusic();
private slots:
	/**
	 * @brief 打印音乐文件
	 */
	void onBtnLoadMusic();
	/**
	 * @brief 开始/暂停音乐播放
	 */
	void onBtnPlayMusic();
	/**
	 * @brief 停止音乐播放
	 */
	void onBtnStopMusic();
	/**
	 * @brief 音乐文件加载状态
	 */
	void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
	/**
	 * @brief 音乐播放状态
	 */
	void onMediaStateChanged(QMediaPlayer::State state);
	/**
	 * @brief 音乐播放器错误
	 */
	void onMediaError(QMediaPlayer::Error error);
	/**
	 * @brief 音乐播放进度
	 */
	void onMediaPositionChanged(qint64 position);
	/**
	 * @brief 音乐总时长
	 */
	void onMediaDurationChanged(qint64 duration);
	/**
	 * @brief 音乐滑块按下
	 */
	void onSliderPressed();
	/**
	 * @brief 音乐滑块释放
	 */
	void onSliderReleased();
private:
	bool eventFilter(QObject* watched, QEvent* event);
signals:
	void sigUpdateMusic(QString file);
	void sigMsuicTime(unsigned int second);
	void playeState(qint8 state);
	void updateMusicWaveFinished();
	/**
	 * @brief 音乐播放总时长变化 秒
	 */
	void sigUpdateMusicTime(unsigned int second);
private:
	Ui::MusicPlayer ui;
	//当前音乐文件
	QString m_qstrMusicFile;
	//音乐播放器
	QMediaPlayer m_mediaPlayer;
	bool m_bPlayIng;
};
