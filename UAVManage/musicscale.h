#pragma once

#include <QWidget>

class MusicScale  : public QWidget
{
	Q_OBJECT

public:
	MusicScale(QWidget *parent);
	~MusicScale();
	/**
	 * @brief 更新音乐播放总时长 毫秒
	 */
	void updateScale(qint64 duration);
	/**
	 * @brief 设置当前播放进度 毫秒
	 */
	void setCurrentPosition(qint64 position);
protected:
	void paintEvent(QPaintEvent* event);
signals:
	void updateMaxWidget(int widget);
private:
	bool m_bUpdateScale;
	qint64 m_nduration;
	qint64 m_nCurrent;
};
