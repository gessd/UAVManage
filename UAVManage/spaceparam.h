#pragma once

#include <QDialog>
#include "ui_spaceparam.h"

class SpaceParam : public QDialog
{
	Q_OBJECT

public:
	SpaceParam(bool init, QWidget *parent = nullptr);
	~SpaceParam();
	unsigned int getSpaceX();
	unsigned int getSpaceY();
	void setProjectPath(QString path);
	void setSpaceSize(unsigned int x, unsigned int y);

private:
	Ui::SpaceParamClass ui;
};
