#pragma once

#include "mavlink_types.h"
#include "common/mavlink.h"

//�豸ϵͳID
#define _DeviceSYS_ID_  1
//�豸���ID
#define _DeviceCOMP_ID_ 1
//��Ϣ�ط���ʶ
#define _NavResendFlag_ 1

//��ָͣ����ϢID
#define _NavStopFlyMessageID_ 15
//�豸TCP���Ӷ˿�
#define _DevicePort_ 8899
//������Ϣʱ����
#define _DeviceHeartbeatInterval_ 500

//ָ����Ϣ�ط����ʹ���
#define _NavLinkResendNum_ 3
//ָ����Ϣ�ط���� ����
#define _NkCommandResendInterval_ 3000

//�·����㳬ʱ ����
#define _NavWaypointTimeout_ 2000
//��ͣ��ʱ���
#define _NavQuickStopFlyTimeout_ 1000
//�����ظ�����
#define _NavWaypointRetryNum_ 3

