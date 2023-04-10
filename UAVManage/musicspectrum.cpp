#include "musicspectrum.h"
#include <QPainter>
#include <QMouseEvent>

MusicSpectrum::MusicSpectrum(QWidget *parent)
	: QWidget(parent)
{
	setMouseTracking(true);
	m_listAudioBuffer.clear();
	m_listSpectrumData.clear();
	m_nMusicTime = 0;
	m_pLaberTime = new QLabel(this);
	m_pLaberTime->close();
	setAttribute(Qt::WA_TranslucentBackground);
	connect(&m_audioDcoder, &QAudioDecoder::finished, this, &MusicSpectrum::onDecoderFinished);
	connect(&m_audioDcoder, &QAudioDecoder::bufferReady, this, &MusicSpectrum::onDecoderBufferReady);
	connect(&m_audioDcoder, SIGNAL(error(QAudioDecoder::Error)), this, SLOT(onDecoderError(QAudioDecoder::Error)));
}

MusicSpectrum::~MusicSpectrum()
{}

void MusicSpectrum::addMusicFile(QString file, unsigned int time)
{
	m_nMusicTime = time;
	m_listAudioBuffer.clear();
	m_listSpectrumData.clear();
	//解码音乐文件，计算波形
	if (QAudioDecoder::StoppedState != m_audioDcoder.state()) {
		m_audioDcoder.stop();
	}
	if (!file.isEmpty()) {
		m_audioDcoder.setSourceFilename(file);
		m_audioDcoder.start();
	}
}

void MusicSpectrum::onDecoderBufferReady()
{
	QAudioBuffer aBuffer = m_audioDcoder.read();
	if (aBuffer.isValid()) m_listAudioBuffer.append(aBuffer);
}

void MusicSpectrum::onDecoderError(QAudioDecoder::Error error)
{
	qDebug() << "音乐解码错误" << error;
}

void MusicSpectrum::onDecoderFinished()
{
	if (0 == m_listAudioBuffer.count()) return;
	qint64 duration = 0;                                //缓冲区时长
	qint64 frameCount = 0;                              //缓冲区帧数
	qint64 byteCount = 0;                               //缓冲区大小
	QAudioFormat format = m_listAudioBuffer[0].format();
	//qDebug() << "音乐文件格式" << format;
	foreach(QAudioBuffer buffer, m_listAudioBuffer) {
		byteCount += buffer.byteCount();
		frameCount += buffer.frameCount();
		duration += buffer.duration();
		int nn = pcmToDB(buffer.data<const char>(), buffer.byteCount());
		m_listSpectrumData.append(nn);
	}
	//qDebug() << "音乐文件大小" << byteCount / 1024.0 << "音乐时长" << duration / 1000.0 / 1000.0 << "总帧数" << frameCount;
	update();
	emit updateMusicWaveFinished();
}

void MusicSpectrum::mouseMoveEvent(QMouseEvent* event)
{
	if (m_nMusicTime > 0) {
		QPoint point = event->pos();
		int x = point.x();
		if (x + m_pLaberTime->width() > this->width()) x = x - m_pLaberTime->width();
		m_pLaberTime->setFixedWidth(70);
		m_pLaberTime->move(QPoint(x + 10, point.y() - 20));
		double time = point.x() * m_nMusicTime / this->width();
		//qDebug() << "当前时间" << time << point.x() << m_nMusicTime << this->width();
		QString strMic, strSec, strMin; //豪秒，秒，分钟字符串
		int mic, secs, mins;			//豪秒，秒，分钟
		mic = time;						//豪秒
		secs = time / 1000;				//秒
		mins = secs / 60;				//分钟
		mic = mic % 1000;				//余数微秒
		secs = secs % 60;				//余数秒
		strMin = mins < 10 ? QString::asprintf("0%1").arg(QString::number(mins)) : QString::number(mins);
		strSec = secs < 10 ? QString::asprintf("0%1").arg(QString::number(secs)) : QString::number(secs);
		strMic = mic < 100 ? "0" : strMic;
		strMic = mic < 10 ? "00" : strMic;
		strMic.append(QString::number(mic));
		QString durationTime = QString::asprintf("%1:%2:%3").arg(strMin).arg(strSec).arg(strMic);
		m_pLaberTime->setText(durationTime);
	}
}

void MusicSpectrum::enterEvent(QEvent* event)
{
	if (m_nMusicTime > 0) {
		m_pLaberTime->setFixedWidth(70);
		m_pLaberTime->show();
		m_pLaberTime->setText("00:00");
	}
}

void MusicSpectrum::leaveEvent(QEvent* event)
{
	m_pLaberTime->close();
}

void MusicSpectrum::paintEvent(QPaintEvent* event)
{
	drawSpectrum();
}

void MusicSpectrum::drawSpectrum()
{
	int count = m_listSpectrumData.count();
	if (count > 0) {
		//绘制波形图
		QPainter painter(this);
		QPen pen;
		pen.setStyle(Qt::SolidLine);
		pen.setColor("#467FC1");
		pen.setWidth(1);
		painter.setPen(pen);
		QBrush brush;
		brush.setColor("#467FC1");
		brush.setStyle(Qt::SolidPattern);
		painter.setBrush(brush);
		int nw = width();
		int nh = height() - 1;

		int w = 4;
		int t = 2;
		//波形柱数量
		int sum = nw / (w + t);
		while (count < sum) {
			w = w * 2;
			t = t * 2;
			sum = nw / (w + t);
		}
		int n = count / sum;
		int m = count % sum;
		for (int i = 0; i < sum; i++) {
			int y = m_listSpectrumData.at(i * n);
			int x = i * (w + t);
			painter.drawRect(QRect(x, nh - y, w, y));
		}
	}
}

int MusicSpectrum::pcmToDB(const char* pcmdata, int size)
{
	if (size <= 0) return 0;
	short value = 0;
	double sum = 0;
	QByteArray data(pcmdata, size);
	for (int i = 0; i < size; i += 2) {
		short x = 0;
		x = data.at(i + 1);
		x = x << 8;
		x = x | data.at(i);
		sum += qAbs(x);
	}
	short ret = sum * 500.0 / (size * 32767);
	ret = qAbs(ret);
	if (ret >= 100) {
		ret = 100;
	}
	return ret;
}