#include "managertopwidget.h"

ManagerTopWidget::ManagerTopWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.pushButton, &QAbstractButton::clicked, [this]() {
		emit sigPrepareWidget();
		close();
		});
}

ManagerTopWidget::~ManagerTopWidget()
{}

void ManagerTopWidget::addWidget(QWidget* pWidget)
{
	ui.horizontalLayout->addWidget(pWidget);
}

void ManagerTopWidget::removeWidget(QWidget* pWidget)
{
	ui.horizontalLayout->removeWidget(pWidget);
}
