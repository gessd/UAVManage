#pragma once

#include <QDialog>
#include "ui_spaceparam.h"

class SpaceParam : public QDialog
{
	Q_OBJECT

public:
	SpaceParam(QWidget *parent = nullptr);
	~SpaceParam();
	unsigned int getSpaceX();
	unsigned int getSpaceY();

private:
	Ui::SpaceParamClass ui;
};
