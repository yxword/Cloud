//
//  "$Id: Message.h 26645 2011-05-31 00:35:01Z wang_haifeng $"
//
//  Copyright (c)1992-2010, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//					2010-8-12		he_linqiang	Create
//

#ifndef __INCLUDED_DAHUA_NETFRAMEWORK_MESSAGE_H__
#define __INCLUDED_DAHUA_NETFRAMEWORK_MESSAGE_H__

namespace Dahua{
namespace NetFramework{
	
//����NetFrameworkģ�����Ϣ������0x0XYZ��ʾ����С��ʮ��������0x1000

//���м̳���CNetHandler��������Լ���handle_close�����ж����Է��͸���Ϣ
//��ʾ�Լ������������Ѿ���������Ϣ�ĸ�������attachδʹ�á�
#define MSG_OBJECT_CLOSED		0x0001
//��������ߵ���Ϣ��ָʾsocket���쳣�������޷��������͡�
//��Ϣ�ĸ�������attachδʹ�á�
#define MSG_SOCK_EXCEPTION	0x0002
//��class CStreamSender��������ߵ���Ϣ��ָʾ��ǰ���л������Ĵ�С��
//��Ϣ�Ļ�������attach��Ϊ���л�������С��
#define MSG_BUFFER_SIZE			0x0004
//��CMediaStreamReceiver�������StopReceive�������쳣��ֱ�ӵ���Close�󣬿�ܷ��������������Ϣ����ʾ�ɹ�ֹͣ����
//��Ϣ�ĸ�������attachδʹ�á�
#define MSG_TIMER_STOP			0x0005
//��CNFile�෢������ߵ���Ϣ��ָʾ��ǰ�ļ���д
//��Ϣ�ĸ�������attachΪ���л�������С
#define MSG_FILE_WRITE			0x0006
//��CNFile�෢������ߵ���Ϣ��ָʾ��ǰ�ļ��ɶ�
//��Ϣ�ĸ�������attachΪ�ɶ�����������С
#define MSG_FILE_READ			0x0007
//��CNFile�෢������ߵ���Ϣ����ʾ�ļ���д�����쳣��
//��Ϣ�ĸ�������attachδʹ�á�
#define MSG_FILE_EXCEPTION	0x0008
//��class CStreamSender��������ߵ���Ϣ��ָʾ��ǰ�����������Ѿ��������
//��Ϣ�ĸ�������attachδʹ�á�
#define MSG_SEND_FINISH		0x0009

}//namespace NetFramework
}//namespace Dahua
#endif //__INCLUDED_DAHUA_NETFRAMEWORK_MESSAGE_H__
