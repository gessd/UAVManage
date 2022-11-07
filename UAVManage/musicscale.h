#pragma once

#include <QWidget>

class MusicScale  : public QWidget
{
	Q_OBJECT

public:
	MusicScale(QWidget *parent);
	~MusicScale();
	void updateScale(qint64 duration);
protected:
	void paintEvent(QPaintEvent* event);
signals:
	void updateMaxWidget(int widget);
private:
	bool m_bUpdateScale;
	qint64 m_nduration;
};
