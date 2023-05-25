#include "musicplayer.h"
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>
#include <QAbstractButton>
#include <QPainter>
#include <QMessageBox>

MusicPlayer::MusicPlayer(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	ui.labelCurrenSlider->setText("00:00");
	connect(ui.btnLoadMusic, &QAbstractButton::clicked, this, &MusicPlayer::onBtnLoadMusic);
	connect(ui.btnMusciPlay, &QAbstractButton::clicked, this, &MusicPlayer::onBtnPlayMusic);
	connect(ui.btnMusciStop, &QAbstractButton::clicked, this, &MusicPlayer::onBtnStopMusic);
	connect(&m_mediaPlayer, &QMediaPlayer::positionChanged, this, &MusicPlayer::onMediaPositionChanged);
	connect(&m_mediaPlayer, &QMediaPlayer::stateChanged, this, &MusicPlayer::onMediaStateChanged);
	connect(&m_mediaPlayer, &QMediaPlayer::durationChanged, this, &MusicPlayer::onMediaDurationChanged);
	connect(&m_mediaPlayer, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(onMediaError(QMediaPlayer::Error)));
	connect(&m_mediaPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(onMediaStatusChanged(QMediaPlayer::MediaStatus)));
	connect(ui.sliderMusic, &QSlider::sliderPressed, this, &MusicPlayer::onSliderPressed);
	connect(ui.sliderMusic, &QSlider::sliderReleased, this, &MusicPlayer::onSliderReleased);
	connect(ui.widgetSpectrum, SIGNAL(updateMusicWaveFinished()), this, SIGNAL(updateMusicWaveFinished()));
}

MusicPlayer::~MusicPlayer()
{}

void MusicPlayer::clearSound()
{
	m_mediaPlayer.stop();
	ui.widgetSpectrum->addMusicFile("", 0);
	m_qstrMusicFile.clear();
	ui.labelSumTime->setText("00:00");
	ui.labelCurrenSlider->setText("00:00");
	ui.labelMusicName->setText("");
	m_mediaPlayer.setMedia(QMediaContent(QUrl("")));
}

void MusicPlayer::updateLoadMusic(QString filePath)
{
	QFileInfo music(filePath);
	if (false == music.isFile()) return;
	clearSound();
	m_qstrMusicFile = filePath;
	m_mediaPlayer.setMedia(QMediaContent(QUrl(filePath)));
}

QPixmap MusicPlayer::getMusicPixmap()
{
	return ui.widgetSpectrum->grab();
}

QString MusicPlayer::getCurrentMusic()
{
	return m_qstrMusicFile;
}

void MusicPlayer::startPlayMusic()
{
	onBtnPlayMusic();
}

void MusicPlayer::stopPlayMusic()
{
	onBtnStopMusic();
}

void MusicPlayer::onBtnLoadMusic()
{
	QString filter = tr("音频文件(*.mp3 *.wav);;mp3文件(*.mp3);;wav文件(*.wav)"); //文件过滤器
	QString qstrFile = QFileDialog::getOpenFileName(this, tr("选择音频文件"), QStandardPaths::writableLocation(QStandardPaths::MusicLocation), filter);
	if (qstrFile.isEmpty()) return;
	emit sigUpdateMusic(qstrFile);
	//updateLoadMusic(qstrFile);
}

void MusicPlayer::onBtnPlayMusic()
{
	if (m_mediaPlayer.state() == QMediaPlayer::StoppedState){
		m_mediaPlayer.play();
	} else if (m_mediaPlayer.state() == QMediaPlayer::PausedState){
		m_mediaPlayer.play();
	} else {
		m_mediaPlayer.pause();
	}
}

void MusicPlayer::onBtnStopMusic()
{
	m_mediaPlayer.stop();
	emit sigMsuicTime(0);
}

void MusicPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
	if (QMediaPlayer::LoadedMedia == status) {
		if (false == m_mediaPlayer.isAudioAvailable()) {
			QMessageBox::warning(this, tr("错误"), tr("音乐文件无法使用"));
			return;
		}
	}
}

void MusicPlayer::onMediaStateChanged(QMediaPlayer::State state)
{
	if (state == QMediaPlayer::PlayingState) {
		ui.btnMusciPlay->setText("暂停");
		ui.btnMusciPlay->setIcon(QIcon(":/res/images/pausemusic.png"));
		emit playeState(1);
	}
	else if (state == QMediaPlayer::PausedState) {
		ui.btnMusciPlay->setText("播放");
		ui.btnMusciPlay->setIcon(QIcon(":/res/images/playmusic.png"));
		emit playeState(2);
	}
	else {
		ui.btnMusciPlay->setText("播放");
		ui.btnMusciPlay->setIcon(QIcon(":/res/images/playmusic.png"));
		ui.sliderMusic->setValue(0);
		ui.labelCurrenSlider->setText("00:00");
		emit playeState(3);
	}
}

void MusicPlayer::onMediaError(QMediaPlayer::Error error)
{
	qDebug() << "音乐播放器错误" << error;
	QMessageBox::warning(this, tr("错误"), tr("音乐文件无法使用"));
}

void MusicPlayer::onMediaPositionChanged(qint64 position)
{
	//音乐当前进度
	ui.sliderMusic->setSliderPosition(position);
	QString strSec, strMin; //豪秒，秒，分钟字符串
	int   secs = position / 1000;	//秒
	int   mins = secs / 60;			//分钟
	secs = secs % 60;				//余数秒
	strMin = mins < 10 ? QString::asprintf("0%1").arg(QString::number(mins)) : QString::number(mins);
	strSec = secs < 10 ? QString::asprintf("0%1").arg(QString::number(secs)) : QString::number(secs);
	QString positionTime = QString::asprintf("%1:%2").arg(strMin).arg(strSec);
	ui.labelCurrenSlider->setText(positionTime);
	unsigned int currentsecs = 0;
	if (currentsecs != mins * 60 + secs) {
		currentsecs = mins * 60 + secs;
		emit sigMsuicTime(currentsecs);
	}
}

void MusicPlayer::onMediaDurationChanged(qint64 duration)
{
	//音乐总时长
	//qDebug() << "音乐文件总时长" << duration;
	if (0 == duration) return;
	if (duration / 1000 > 5 * 60) {
		ui.widgetSpectrum->addMusicFile("", 0);
		m_qstrMusicFile.clear();
		ui.labelSumTime->setText("00:00");
		ui.labelCurrenSlider->setText("00:00");
		ui.labelMusicName->setText("");
		QMessageBox::warning(this, tr("警告"), tr("音乐时长超过5分钟"));
		m_mediaPlayer.setMedia(QMediaContent(QUrl("")));
		return;
	}
	QFileInfo info(m_qstrMusicFile);
	ui.labelMusicName->setText(info.fileName());
	ui.sliderMusic->setMaximum(duration);
	QString strSec, strMin;					//豪秒，秒，分钟字符串
	unsigned int sum = duration / 1000;	//秒
	unsigned int mins = sum / 60;			//分钟
	unsigned int secs = sum % 60;						//余数秒
	strMin = mins < 10 ? QString::asprintf("0%1").arg(QString::number(mins)) : QString::number(mins);
	strSec = secs < 10 ? QString::asprintf("0%1").arg(QString::number(secs)) : QString::number(secs);
	QString durationTime = QString::asprintf("%1:%2").arg(strMin).arg(strSec);
	ui.labelSumTime->setText(durationTime);
	ui.widgetSpectrum->addMusicFile(m_qstrMusicFile, duration);
	emit sigUpdateMusicTime(sum);
}

void MusicPlayer::onSliderPressed()
{
	if (m_mediaPlayer.state() == QMediaPlayer::PlayingState) m_mediaPlayer.pause();
}

void MusicPlayer::onSliderReleased()
{
	if (m_qstrMusicFile.isEmpty()) {
		ui.sliderMusic->setValue(0);
		return;
	}
	qint64 position = ui.sliderMusic->value();
	m_mediaPlayer.setPosition(position);
	m_mediaPlayer.play();
}
