#include "uavmanage.h"

UAVManage::UAVManage(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    //connect(ui.btnTest, SIGNAL(clicked()), this, SLOT(onBtnTestClicked()));
    m_pDeviceManage = new DeviceManage(this);
    ui.gridLayout->addWidget(m_pDeviceManage, 0,1);
}

void UAVManage::onBtnTestClicked()
{
    
}
