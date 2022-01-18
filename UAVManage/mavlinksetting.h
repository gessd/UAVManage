#pragma once

#include "mavlink_types.h"
#include "common/mavlink.h"

//�豸ϵͳID
#define _DeviceSYS_ID_  1
//�豸���ID
#define _DeviceCOMP_ID_ 1
//��Ϣ�ط���ʶ
#define _MavResendFlag_ 1

//�豸TCP���Ӷ˿�
#define _DevicePort_ 8899

//��ָͣ����ϢID
#define _MavStopFlyMessageID_ 15
//��ͣ��ʱ���
#define _MavQuickStopFlyTimeout_ 1000
//ָ����Ϣ�ط����ʹ���
#define _MavLinkResendNum_ 2
//�����ظ�����
#define _MavWaypointRetryNum_ 2
//������Ϣʱ����
#define _DeviceHeartbeatInterval_ 500
//ָ����Ϣ�ط���� ����
#define _NkCommandResendInterval_ 3000
//�·����㳬ʱ ����
#define _MavWaypointTimeout_ 2000
