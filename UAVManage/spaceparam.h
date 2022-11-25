#pragma once

#include <QDialog>
#include "ui_spaceparam.h"

class SpaceParam : public QDialog
{
	Q_OBJECT

public:
	SpaceParam(bool init, QWidget *parent);
	~SpaceParam();
	unsigned int getSpaceX();
	unsigned int getSpaceY();
	void setProjectPath(QString path);
	void setSpaceSize(unsigned int x, unsigned int y);
protected:
	void showEvent(QShowEvent* event);
	void hideEvent(QHideEvent* event);
private:
	Ui::SpaceParamClass ui;
	QLabel* m_pLabelBackground;
};
