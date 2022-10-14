#pragma once

#include <QDialog>
#include "ui_spaceparam.h"

class SpaceParam : public QDialog
{
	Q_OBJECT

public:
	SpaceParam(QWidget *parent = nullptr);
	~SpaceParam();
	int getSpaceX();
	int getSpaceY();

private:
	Ui::SpaceParamClass ui;
};
