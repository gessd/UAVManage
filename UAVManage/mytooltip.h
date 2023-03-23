#pragma once
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPoint>
#include <QEvent>
class MyTooltip : public QLabel
{
    Q_OBJECT
public:
	enum MyEvent {
		MyToolTipEvent = QEvent::User + 1021,
	};
    MyTooltip(QWidget* parent);
    void onShow(const QPoint& point);
    void onHide();
    void setText(const QString& text);
protected:
    bool event(QEvent* e) override;
    void showEvent(QShowEvent* e) override;
    QString m_qstrLast;
    bool m_bMove;
};

