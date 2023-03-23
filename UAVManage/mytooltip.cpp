#include "mytooltip.h"
#include <QEvent>
#include <QHelpEvent>
#include <QDebug>

MyTooltip::MyTooltip(QWidget* parent)
    :QLabel(parent)
{
    m_bMove = false;
    setFixedHeight(22);
    setWindowFlags(Qt::ToolTip);    
    setAlignment(Qt::AlignCenter);
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("background:#FFFFFF;border-radius:0px;border:1px solid #767676;color:#767676;font-size:14px;");
    onHide();
}

void MyTooltip::onShow(const QPoint& pos)
{
    setVisible(true);
    if (m_bMove) move(pos);
}

void MyTooltip::onHide()
{
    setVisible(false);
}

void MyTooltip::setText(const QString& text)
{
    m_bMove = false;
    if (m_qstrLast != text) {
        onHide();
        m_bMove = true;
    }
    m_qstrLast = text;
    QLabel::setText(text);
}

bool MyTooltip::event(QEvent* e)
{
    if (e->type() == MyToolTipEvent) {
        auto e_ = static_cast<QHelpEvent*>(e);
        auto pos = e_->globalPos();
        onShow(QPoint(pos.x() + 5, pos.y() + 10));
        e->accept();
    }
    return __super::event(e);
}

void MyTooltip::showEvent(QShowEvent* e)
{
    adjustSize();
    //setMask(QRegion(rect(),QRegion::Ellipse));
}
