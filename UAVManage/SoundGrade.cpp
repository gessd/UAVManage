#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTime>
#include "SoundGrade.h"
const int PATH_ROLE = Qt::UserRole + 1000;
const int ONTIME_INTERVAL = 1000;

SoundGrade::SoundGrade(QWidget *parent) :QWidget(parent)
{
	m_ui.setupUi(this);
	player = NULL;
	player = new QMediaPlayer(this);                                //初始化播放器
	player->setNotifyInterval(10);
	playlist = new QMediaPlaylist(this);                            //初始化播放器列表
	playlist->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);     //播放模式：仅播放一次
	player->setPlaylist(playlist);                                  //设置播放列表
	signalConnectSlot();

	//TODO暂时隐藏
	m_ui.m_pBtnPrev->setVisible(false);
	m_ui.m_pPlayList->setVisible(false);
	m_ui.m_pBtnNext->setVisible(false);	

	m_ui.m_pBtnPlay->setEnabled(false);
	m_ui.m_pbtnStop->setEnabled(false);
	connect(m_ui.m_pbtnStop, &QAbstractButton::clicked, [this]() {stopPlayMusic(); });
}

void SoundGrade::signalConnectSlot()
{//绑定信号槽
	connect(player, SIGNAL(stateChanged(QMediaPlayer::State)),   //播放状态改变
		this, SLOT(onStateChanged(QMediaPlayer::State)));

	connect(player, SIGNAL(positionChanged(qint64)),             //播放位置改变
		this, SLOT(onPositionChanged(qint64)));

	connect(player, SIGNAL(durationChanged(qint64)),             //持续时间改变
		this, SLOT(onDurationChanged(qint64)));
}

SoundGrade::~SoundGrade()
{
	stopPlayMusic();
}

void SoundGrade::updateLoadMusic(QString filePath)
{
	if (!QFile::exists(filePath)) return;
	playlist->removeMedia(0, playlist->mediaCount() - 1);
	playlist->addMedia(QUrl::fromLocalFile(filePath));
	playlist->setCurrentIndex(m_ui.m_pPlayList->count() - 1);
	m_decoder = new AudioDecoder(this);
	connect(m_decoder, SIGNAL(finished()), this, SLOT(onDecoderFinished()));
	m_decoder->analysisAudioFile(filePath);
	QFileInfo info(filePath);
	QString name = info.fileName();
	m_ui.labelMusicName->setText(name);
	m_ui.m_pBtnPlay->setEnabled(true);
	m_ui.m_pbtnStop->setEnabled(true);
}

void SoundGrade::startPlayMusic()
{
	stopPlayMusic();
	on_m_pBtnPlay_clicked();
}

void SoundGrade::stopPlayMusic()
{
	if(player) player->stop();
}

void SoundGrade::on_m_pBtnPlay_clicked()
{//响应播放按钮

	if (player->state() == QMediaPlayer::StoppedState)
	{
		if (playlist->currentIndex() < 0)
			playlist->setCurrentIndex(0);
		//解码当前播放文件
		QMediaContent aContent = playlist->currentMedia();
		QUrl aUrl = aContent.canonicalUrl();
		QString fileName = aUrl.path();
		fileName = fileName.right(fileName.size() - 1);
		//m_decoder = new AudioDecoder(this);
		//connect(m_decoder, SIGNAL(finished()),                       //解码器解码成功后更新界面
		//	this, SLOT(onDecoderFinished()));
		//m_decoder->analysisAudioFile(fileName);
		player->play();
		emit startPlay();

		m_bPlaying = true;
	}
	else if (player->state() == QMediaPlayer::PausedState)
	{
		player->play();
		m_bPlaying = true;
	}
	else
	{
		player->pause();
		m_bPlaying = false;
	}
}

void SoundGrade::on_m_pBtnLoad_clicked()
{//添加文件按钮
	QString curPath = QDir::homePath();			//获取系统当前目录
	QString dlgTitle = QString("选择音频文件");			//对话框标题
	QString filter = QString("音频文件(*.mp3 *.wav *.wma);;mp3文件(*.mp3);;wav文件(*.wav);;wma文件(*.wma);;所有文件(*.*)"); //文件过滤器

	//TODO改为使用单独一个音乐的方式
	QString qstrFile = QFileDialog::getOpenFileName(this, dlgTitle, "", filter);
	if (qstrFile.isEmpty()) return;
	updateLoadMusic(qstrFile);
	emit sigUpdateMusic(qstrFile);
}

void SoundGrade::on_m_pPlayList_currentRowChanged(int currentRow)
{//单击播放列表
	playlist->setCurrentIndex(currentRow);
}

void SoundGrade::on_m_pPlayList_doubleClicked(const QModelIndex &index)
{//双击播放列表
	if (player->state() != QMediaPlayer::State::StoppedState)
		player->stop();
	int rowNo = index.row();
	playlist->setCurrentIndex(rowNo);
}

void SoundGrade::on_m_pBtnPrev_clicked()
{//上一首
	if (player->state() != QMediaPlayer::State::StoppedState)
		player->stop();
	int Row = m_ui.m_pPlayList->currentRow();
	if (Row > 0)
		m_ui.m_pPlayList->setCurrentRow(--Row);
}
void SoundGrade::on_m_pBtnNext_clicked()
{//下一首
	if (player->state() != QMediaPlayer::State::StoppedState)
		player->stop();
	int Row = m_ui.m_pPlayList->currentRow();
	if (Row < m_ui.m_pPlayList->count() - 1)
		m_ui.m_pPlayList->setCurrentRow(++Row);
}
void SoundGrade::onDecoderFinished()
{//解码成功后槽函数
	bufferList tmpBufferList = m_decoder->getBufferList();
	connect(m_decoder, SIGNAL(finished()),                       //解码器解码成功后更新界面
		this, SLOT(onDecoderFinished()));
	disconnect(m_decoder, SIGNAL(finished()),                    //解除绑定
		this, SLOT(onDecoderFinished()));
	delete m_decoder;
	if (tmpBufferList.isEmpty())
		return;
	AxisAndSeriesNorm tmpNorm;							//轴与序列规范
	qint64 duration = 0;                                //缓冲区时长
	qint64 frameCount = 0;                              //缓冲区帧数
	qint64 byteCount = 0;                               //缓冲区大小
	QAudioBuffer tmpBuffer = tmpBufferList[0];
	QAudioFormat tmpFormat = tmpBuffer.format();        //缓冲区格式
	tmpNorm.ChannelCount = tmpFormat.channelCount();    //通道数
	tmpNorm.SampleSizes = tmpFormat.sampleSize();       //采样大小 采样点位数
	QByteArray AudioByte;                               //储存缓冲数据与绘制数据
	for (auto buffer : tmpBufferList)
	{
		byteCount += buffer.byteCount();
		frameCount += buffer.frameCount();
		duration += buffer.duration();
		AudioByte.append(QByteArray(buffer.data<char>(), buffer.byteCount()));
	}
	qint64 interval = frameCount / 5000;                    //计算显示的帧数间隔
	tmpNorm.frameCount = frameCount;                        //总帧数
	tmpNorm.showFrameCount = 5000;                          //展示帧数
	tmpNorm.drawInterval = interval;                        //计算显示的帧数间隔
	tmpNorm.duration = duration;                            //播放时间
	//m_ui.m_pCuverPlot->clearSeriesPoint();
	m_ui.m_pCuverPlot->resetAxisAndSeries(tmpNorm);
	if (tmpNorm.ChannelCount == 1)
		m_ui.m_pCuverPlot->addSeriesPoint<qint8>((qint8*)AudioByte.data(), tmpNorm.frameCount);
	else if (tmpNorm.ChannelCount == 2)
		m_ui.m_pCuverPlot->addSeriesPoint<qint16>((qint16*)AudioByte.data(), tmpNorm.frameCount);
	
}

void SoundGrade::onStateChanged(QMediaPlayer::State state)
{//播放器状态变化，更新按钮状态

	if (state == QMediaPlayer::PlayingState)
	{
		m_ui.m_pBtnPlay->setText(QStringLiteral("暂停"));
		m_ui.m_pBtnPlay->setIcon(QIcon(":/res/images/pausemusic.png"));
		emit isPlaying(true);
		emit playeState(1);
	}
	else if (state == QMediaPlayer::PausedState)
	{
		m_ui.m_pBtnPlay->setText(QStringLiteral("播放"));
		m_ui.m_pBtnPlay->setIcon(QIcon(":/res/images/playmusic.png"));
		emit isPlaying(false);
		emit playeState(2);
	}
	else
	{
		m_ui.m_pBtnPlay->setText(QStringLiteral("播放"));
		m_ui.m_pBtnPlay->setIcon(QIcon(":/res/images/playmusic.png"));
		emit isPlaying(false);
		emit playeState(3);
	}
}


void SoundGrade::onDurationChanged(qint64 duration)
{//文件时长变化，更新进度显示
	m_ui.m_pHslider->setMaximum(duration);
	QString strSec, strMin; //豪秒，秒，分钟字符串
	int   secs = duration / 1000;	//秒
	int   mins = secs / 60;			//分钟
	secs = secs % 60;				//余数秒
	strMin = mins < 10 ? QString::asprintf("0%1").arg(QString::number(mins)) : QString::number(mins);
	strSec = secs < 10 ? QString::asprintf("0%1").arg(QString::number(secs)) : QString::number(secs);
	durationTime = QString::asprintf("%1:%2").arg(strMin).arg(strSec);
	m_ui.m_pLblTotal->setText(durationTime);
}

void SoundGrade::onPositionChanged(qint64 position)
{//当前文件播放位置变化，更新进度显示
	if (m_ui.m_pHslider->isSliderDown())
		return;
	QString strSec, strMin; //豪秒，秒，分钟字符串
	m_ui.m_pHslider->setSliderPosition(position);//
	m_ui.m_pCuverPlot->axisXPlaying(position * 1000);
	int   secs = position / 1000;	//秒
	int   mins = secs / 60;			//分钟
	secs = secs % 60;				//余数秒
	strMin = mins < 10 ? QString::asprintf("0%1").arg(QString::number(mins)) : QString::number(mins);
	strSec = secs < 10 ? QString::asprintf("0%1").arg(QString::number(secs)) : QString::number(secs);
	positionTime = QString::asprintf("%1:%2").arg(strMin).arg(strSec);
	m_ui.m_pLblCur->setText(positionTime);
	static int currentsecs = 0;
	if (currentsecs != mins * 60 + secs) {
		currentsecs = mins * 60 + secs;
		emit sigMsuicTime(currentsecs);
	}
}

void SoundGrade::on_m_pHslider_sliderPressed()
{//开始拖动滑块时暂停音频播放
	m_sliderPlaying = m_bPlaying;
	if (player->state() == QMediaPlayer::PlayingState)
		player->pause();
}

void SoundGrade::on_m_pHslider_sliderReleased()
{//拖动滑块后进行音频播放进度调控
	qint64 position = m_ui.m_pHslider->value();
	player->setPosition(position);
	if (m_sliderPlaying)
		player->play();
}