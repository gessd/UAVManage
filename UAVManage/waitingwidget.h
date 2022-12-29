#ifndef WaitingWidget_H
#define WaitingWidget_H

#include <QWidget>
#include <QMovie>

namespace Ui {
class WaitingWidget;
}

class WaitingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WaitingWidget(QWidget *parent);
    ~WaitingWidget();
    void visibleWidget();
private:
    Ui::WaitingWidget *ui;
};

#endif // WaitingWidget_H
