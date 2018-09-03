/****************************************************************************************
Author: yxs
Date: 2018-07-06
Description:微软语音识别接口封装
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
	Description: 初始化语音识别
	Input:
	sRobotName，机器人名，对部分识别的命令有用，为空时，表示识别跳舞，不为空 sRobotName + 跳舞
	Output:
	Return :   0 成功  -1 失败
	Others :
	*************************************************/
	int Init(const std::string sRobotName = "");

	/*************************************************
	Description: 反初始化语音识别
	Input:
	Output:
	Return :   0 成功  -1 失败
	Others :
	*************************************************/
	int Uninit();

	/*************************************************
	Description: 语音识别回调处理
	Input:
	mssr_cb，识别处理回调函数
	Output:
	Return :   0 成功  -1 失败
	Others :
	*************************************************/
	int SetMSSRCb(CallbackFun mssr_cb);
	
	/*************************************************
	Description: 声音开始回调处理
	Input:
	mssr_cb，声音开始回调
	Output:
	Return :   0 成功  -1 失败
	Others :
	*************************************************/
	int SetMSSRSoundStart(CallbackFunSoundStart mssr_sound_start);

	/*************************************************
	Description: 声音结束回调处理
	Input:
	mssr_cb，声音开始回调
	Output:
	Return :   0 成功  -1 失败
	Others :
	*************************************************/
	int SetMSSRSoundEnd(CallbackFunSoundEnd mssr_sound_end);

	/*************************************************
	Description: 启动语音识别
	Input:
	Output:
	Return :   0 成功  -1 失败
	Others :
	*************************************************/
	int Start();

	/*************************************************
	Description: 关闭语音识别
	Input:
	Output:
	Return :   0 成功  -1 失败
	Others :
	*************************************************/
	int Stop();

	/*************************************************
	Description: 获取机器人命令
	Input:
	lsVoiceCtrl:命令值和命令名称键值对队列，如：<0，MOVE_FRONT>
	lsRobotCtrl:命令语音文本和命令值，如<xy向前进，0>
	Output:
	Return :   0 成功  -1 失败
	Others :
	*************************************************/
	int GetCtrlCmd(const std::list<std::pair<std::string, int > >& lsVoiceCtrl,std::list<std::pair<std::string, int > >&lsRobotCtrl);
	int GetRobotCtrl(SLocalCmd* pInParam,int nInLen,SLocalCmd* pOutParam,int nOutLen);

protected:
	/*************************************************
	Description: 语音识别线程处理函数
	Input:
	pParam，识别线程传入参数
	Output:
	Return :   0 成功  -1 失败
	Others :
	*************************************************/
	static unsigned __stdcall StartReco(void* pParam);

	/*************************************************
	Description: 阻塞等待语音识别结果
	Input:
	ppResult,识别结果
	Output:
	Return :   0 成功  -1 失败
	Others :
	*************************************************/
	HRESULT BlockWaitForReco(ISpRecoResult ** ppResult);

	/*************************************************
	Description: 枚举所有的语音包
	Input:
	ppResult：识别结果
	Output:
	Return :   0 成功  -1 失败
	Others :
	*************************************************/
	int EnumSRObject();

	/*************************************************
	Description: 语音识别文本处理
	Input:
	str：播报的内容
	bRecoRbtName，表示是否是基于机器人名的识别
	Output:
	Return :   0 成功  -1 失败
	Others :
	*************************************************/
	int DoRecoText(const char* str,bool bRecoRbtName);

	/*************************************************
	Description: 读取配置文件有关微软离线语音识别内容
	Input:
	Output:
	Return :   0 成功  -1 失败
	Others :
	*************************************************/
	int GetConfig();

	/*************************************************
	Description: 将识别命令加上首字符串（机器人名字）
	Input:
	sStr:首字符串
	Output:
	Return :   0 成功  -1 失败
	Others :
	*************************************************/
	int FormatCmdXMLFile(std::string sStr);

private:
	HWND m_hWnd;
	bool m_bLoadCmdGrammer;
	std::string m_cmdXmlFilePath;

protected:
	CComPtr<ISpRecognizer>	m_cpRecoEngine;	// 语音识别引擎(recognition)的接口。
	CComPtr<ISpObjectToken>	m_cpObjectToken; // 
	CComPtr<ISpAudio> m_cpAudio;
	CComPtr<ISpRecoContext> m_cpRecoCtxt;	// 识别引擎上下文(context)的接口。
	LANGID  m_langid;
	CComPtr<ISpRecoGrammar> m_cpCmdGrammar;	// 识别文法(grammar)的接口。
	CComPtr<ISpVoice> m_cpVoice;
	ULONGLONG  ullGrammerID;
	CallbackFun m_mssr_cb;
	CallbackFunSoundStart m_mssr_sound_start;
	CallbackFunSoundEnd m_mssr_sound_end;
	bool m_bGotReco;
	HANDLE         m_pThrdreco;
	bool m_bthrd_reco_exit;   // 线程退出标志
	bool m_bInited;
	bool m_bInSound;
	bool m_bRecoRbtName;      // 是否识别带机器人名称的命令
	bool m_bRecord;			// 是否启动本地语音识别

};

#endif