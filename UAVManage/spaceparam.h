#pragma once

#include <QDialog>
#include "ui_spaceparam.h"

//#define _EditSpace_
class SpaceParam : public QDialog
{
	Q_OBJECT

public:
	SpaceParam(bool create, QWidget *parent);
	~SpaceParam();
	unsigned int getSpaceX();
	unsigned int getSpaceY();
	QString getProjectPath();
	void setProjectPath(QString path);
	void setSpaceSize(unsigned int x, unsigned int y);
protected:
	void showEvent(QShowEvent* event);
	void hideEvent(QHideEvent* event);
private:
	Ui::SpaceParamClass ui;
	QLabel* m_pLabelBackground;
};
