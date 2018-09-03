/****************************************************************************************
Author: yxs
Date: 2018-07-06
Description:΢������ʶ��ӿڷ�װ
*****************************************************************************************/
#pragma once
#ifndef _MSSR_H
#define _MSSR_H
#include <windows.h>
#include <string>
#include <list>
#include <sapi.h> 
#include <sphelper.h>
struct SLocalCmd{
	char str[256];
	int nType;
	SLocalCmd(){
		memset(str,0,sizeof(str));
		nType = -1;
	}
};


#pragma comment(lib,"sapi.lib")
typedef int(*CallbackFun)(const char* str,bool bRecoRbtName);
typedef int(*CallbackFunSoundStart)();
typedef int(*CallbackFunSoundEnd)();
using namespace std;

class CMSSR
{
public:
	CMSSR();
	~CMSSR();
public:
	/*************************************************
	Description: ��ʼ������ʶ��
	Input:
	sRobotName�������������Բ���ʶ����������ã�Ϊ��ʱ����ʾʶ�����裬��Ϊ�� sRobotName + ����
	Output:
	Return :   0 �ɹ�  -1 ʧ��
	Others :
	*************************************************/
	int Init(const std::string sRobotName = "");

	/*************************************************
	Description: ����ʼ������ʶ��
	Input:
	Output:
	Return :   0 �ɹ�  -1 ʧ��
	Others :
	*************************************************/
	int Uninit();

	/*************************************************
	Description: ����ʶ��ص�����
	Input:
	mssr_cb��ʶ����ص�����
	Output:
	Return :   0 �ɹ�  -1 ʧ��
	Others :
	*************************************************/
	int SetMSSRCb(CallbackFun mssr_cb);
	
	/*************************************************
	Description: ������ʼ�ص�����
	Input:
	mssr_cb��������ʼ�ص�
	Output:
	Return :   0 �ɹ�  -1 ʧ��
	Others :
	*************************************************/
	int SetMSSRSoundStart(CallbackFunSoundStart mssr_sound_start);

	/*************************************************
	Description: ���������ص�����
	Input:
	mssr_cb��������ʼ�ص�
	Output:
	Return :   0 �ɹ�  -1 ʧ��
	Others :
	*************************************************/
	int SetMSSRSoundEnd(CallbackFunSoundEnd mssr_sound_end);

	/*************************************************
	Description: ��������ʶ��
	Input:
	Output:
	Return :   0 �ɹ�  -1 ʧ��
	Others :
	*************************************************/
	int Start();

	/*************************************************
	Description: �ر�����ʶ��
	Input:
	Output:
	Return :   0 �ɹ�  -1 ʧ��
	Others :
	*************************************************/
	int Stop();

	/*************************************************
	Description: ��ȡ����������
	Input:
	lsVoiceCtrl:����ֵ���������Ƽ�ֵ�Զ��У��磺<0��MOVE_FRONT>
	lsRobotCtrl:���������ı�������ֵ����<xy��ǰ����0>
	Output:
	Return :   0 �ɹ�  -1 ʧ��
	Others :
	*************************************************/
	int GetCtrlCmd(const std::list<std::pair<std::string, int > >& lsVoiceCtrl,std::list<std::pair<std::string, int > >&lsRobotCtrl);
	int GetRobotCtrl(SLocalCmd* pInParam,int nInLen,SLocalCmd* pOutParam,int nOutLen);

protected:
	/*************************************************
	Description: ����ʶ���̴߳�����
	Input:
	pParam��ʶ���̴߳������
	Output:
	Return :   0 �ɹ�  -1 ʧ��
	Others :
	*************************************************/
	static unsigned __stdcall StartReco(void* pParam);

	/*************************************************
	Description: �����ȴ�����ʶ����
	Input:
	ppResult,ʶ����
	Output:
	Return :   0 �ɹ�  -1 ʧ��
	Others :
	*************************************************/
	HRESULT BlockWaitForReco(ISpRecoResult ** ppResult);

	/*************************************************
	Description: ö�����е�������
	Input:
	ppResult��ʶ����
	Output:
	Return :   0 �ɹ�  -1 ʧ��
	Others :
	*************************************************/
	int EnumSRObject();

	/*************************************************
	Description: ����ʶ���ı�����
	Input:
	str������������
	bRecoRbtName����ʾ�Ƿ��ǻ��ڻ���������ʶ��
	Output:
	Return :   0 �ɹ�  -1 ʧ��
	Others :
	*************************************************/
	int DoRecoText(const char* str,bool bRecoRbtName);

	/*************************************************
	Description: ��ȡ�����ļ��й�΢����������ʶ������
	Input:
	Output:
	Return :   0 �ɹ�  -1 ʧ��
	Others :
	*************************************************/
	int GetConfig();

	/*************************************************
	Description: ��ʶ������������ַ��������������֣�
	Input:
	sStr:���ַ���
	Output:
	Return :   0 �ɹ�  -1 ʧ��
	Others :
	*************************************************/
	int FormatCmdXMLFile(std::string sStr);

private:
	HWND m_hWnd;
	bool m_bLoadCmdGrammer;
	std::string m_cmdXmlFilePath;

protected:
	CComPtr<ISpRecognizer>	m_cpRecoEngine;	// ����ʶ������(recognition)�Ľӿڡ�
	CComPtr<ISpObjectToken>	m_cpObjectToken; // 
	CComPtr<ISpAudio> m_cpAudio;
	CComPtr<ISpRecoContext> m_cpRecoCtxt;	// ʶ������������(context)�Ľӿڡ�
	LANGID  m_langid;
	CComPtr<ISpRecoGrammar> m_cpCmdGrammar;	// ʶ���ķ�(grammar)�Ľӿڡ�
	CComPtr<ISpVoice> m_cpVoice;
	ULONGLONG  ullGrammerID;
	CallbackFun m_mssr_cb;
	CallbackFunSoundStart m_mssr_sound_start;
	CallbackFunSoundEnd m_mssr_sound_end;
	bool m_bGotReco;
	HANDLE         m_pThrdreco;
	bool m_bthrd_reco_exit;   // �߳��˳���־
	bool m_bInited;
	bool m_bInSound;
	bool m_bRecoRbtName;      // �Ƿ�ʶ������������Ƶ�����
	bool m_bRecord;			// �Ƿ�������������ʶ��

};

#endif