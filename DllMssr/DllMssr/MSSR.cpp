// MsSR.cpp : ���� DLL Ӧ�ó���ĵ���������
//
#include "stdafx.h"
#include "tinyxml2.h"
#include "MSSR.h"
#include "commFunc.h"

CMSSR::CMSSR()
{
	m_bInited = false;
	m_mssr_cb = NULL;
	m_bthrd_reco_exit = false;
	m_pThrdreco = nullptr;
	m_bLoadCmdGrammer = true;
	m_bRecoRbtName = false;
	m_bRecord = false;
}


CMSSR::~CMSSR()
{
	Uninit();
}

int CMSSR::Init(const std::string sRobotName)
{
	if (m_bInited)
	{
		return -1;
	}
	GetConfig();
	if (!sRobotName.empty())
	{
		m_bRecoRbtName = true;
		if(FAILED(FormatCmdXMLFile(sRobotName)))
		{
			////LOGERROR("΢����������ʶ��:����������д����������ļ���ʧ��");
			return -1;
		}
	}
	::CoInitialize(NULL);
	HRESULT hr = E_FAIL;
	hr = m_cpRecoEngine.CoCreateInstance(CLSID_SpInprocRecognizer);
	if (FAILED(hr))
	{
		//LOGERROR("΢����������ʶ��:��ʼ������ʶ������ʧ��");
		return -1;
	}

	hr = SpGetDefaultTokenFromCategoryId(SPCAT_AUDIOIN, &m_cpObjectToken);
	if (FAILED(hr))
	{
		//LOGERROR("΢����������ʶ��:��ȡĬ��Audio ���� Tokenʧ��");
		return -1;
	}

	hr = m_cpRecoEngine->SetInput(m_cpObjectToken, TRUE);
	if (FAILED(hr))
	{
		//LOGERROR("΢����������ʶ��:����Ĭ��Audio ���� Tokenʧ��");
		return -1;
	}

	/*hr = SpCreateDefaultObjectFromCategoryId(SPCAT_AUDIOIN, &m_cpAudio);
	if (FAILED(hr))
	{
		//LOGERROR("����Ĭ������AudioInʧ��:" << hr);
		return -1;
	}

	hr = m_cpRecoEngine->SetInput(m_cpAudio, true);
	if (FAILED(hr))
	{
		//LOGERROR("����Ĭ������AudioInʧ��:" << hr);
		return -1;
	}*/

	/*hr = m_cpRecoEngine->SetInput(NULL, TRUE);
	if (FAILED(hr))
	{
		//LOGERROR("ask the shared recognizer to re-check the default audio input token:" << hr);
		return -1;
	}*/

	SPRECOSTATE spRecoState;
	hr = m_cpRecoEngine->GetRecoState(&spRecoState);
	if (FAILED(hr))
	{
		//LOGERROR("΢����������ʶ��:��ȡ����ʶ�������״̬ʧ��");
		return -1;
	}
	if (spRecoState == SPRST_INACTIVE)
	{
		hr = m_cpRecoEngine->SetRecoState(SPRST_ACTIVE);
		if (FAILED(hr))
		{
			//LOGERROR("΢����������ʶ��:��������ʶ������״̬ΪACTIVE");
			return -1;
		}
	}

	hr = m_cpRecoEngine->CreateRecoContext(&m_cpRecoCtxt);
	if (FAILED(hr))
	{
		//LOGERROR("΢����������ʶ��:��������ʶ��������ʧ��");
		return -1;
	}

	hr = m_cpRecoCtxt->GetVoice(&m_cpVoice);
	if (FAILED(hr))
	{
		//LOGERROR("΢����������ʶ��:��ȡ������������ʧ��");
		return -1;
	}
	//EnumSRObject();
	hr = m_cpRecoCtxt->SetNotifyWin32Event();
	if (FAILED(hr))
	{
		//LOGERROR("΢����������ʶ��:����win32�¼�֪ͨʧ��");
		return -1;
	}
	const ULONGLONG ullInterest = /*SPFEI(SPEI_SOUND_START) | SPFEI(SPEI_SOUND_END) |*/
		SPFEI(SPEI_RECOGNITION) |SPFEI(SPEI_FALSE_RECOGNITION) /*| SPFEI(SPEI_INTERFERENCE)*/ |
		SPFEI(SPEI_RECO_OTHER_CONTEXT) /*|SPFEI(SPEI_REQUEST_UI)*/ | SPFEI(SPEI_RECO_STATE_CHANGE);
	hr = m_cpRecoCtxt->SetInterest(ullInterest, ullInterest);
	if (FAILED(hr))
	{
		//LOGERROR("΢����������ʶ��:���ø���Ȥ�¼�ʧ��");
		return -1;
	}

	hr = m_cpRecoCtxt->SetMaxAlternates(3);
	/*SPRECOGNIZERSTATUS stat;
	ZeroMemory(&stat, sizeof(stat));
	hr = m_cpRecoEngine->GetStatus(&stat);

	m_langid = stat.aLangID[0];
	if (m_langid != MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US))
	{

	}*/
	/*hr = m_cpRecoCtxt->SetAudioOptions(SPAO_RETAIN_AUDIO, NULL, NULL);
	if (FAILED(hr))
	{
		//LOGERROR("SetAudioOptionsʧ��:" << hr);
		return -1;
	}*/
	if (m_bLoadCmdGrammer)
	{
		ULONGLONG  ullGrammerID = 1000;
		WCHAR wszXMLFile[256] = L"";
		MultiByteToWideChar(CP_ACP, 0, m_cmdXmlFilePath.c_str(), -1, wszXMLFile, 256);
		hr = m_cpRecoCtxt->CreateGrammar(ullGrammerID, &m_cpCmdGrammar);
		if (FAILED(hr))
		{
			//LOGERROR("΢����������ʶ��:�����﷨����ʧ��");
			return -1;
		}

		/*hr = m_cpRecoCtxt->SetAudioOptions(SPAO_RETAIN_AUDIO, NULL, NULL);
		if (FAILED(hr))
		{
		//LOGERROR("���ñ�����Ƶʧ��:" << hr);
		return -1;
		}*/

		hr = m_cpCmdGrammar->LoadCmdFromFile(wszXMLFile, SPLO_DYNAMIC);
		if (FAILED(hr))
		{
			//LOGERROR("΢����������ʶ��:�����﷨�ļ�ʧ��");
			return -1;
		}

		hr = m_cpCmdGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);
		if (FAILED(hr))
		{
			//LOGERROR("΢����������ʶ��:����SetRuleStateʧ��");
			return -1;
		}
	}
	else
	{
		hr = m_cpRecoCtxt->CreateGrammar(0, &m_cpCmdGrammar);
		if (FAILED(hr))
		{
			//LOGERROR("΢����������ʶ��:���������﷨ʧ��");
			return -1;
		}
		hr = m_cpCmdGrammar->LoadDictation(NULL, SPLO_STATIC);
		if (FAILED(hr))
		{
			//LOGERROR("΢����������ʶ��:���������﷨ʧ��");
			return -1;
		}
		hr = m_cpCmdGrammar->SetDictationState(SPRS_ACTIVE);
		if (FAILED(hr))
		{
			//LOGERROR("΢����������ʶ��:��������״̬ʧ��");
			return -1;
		}
	}
	
	// ��������ʶ����߳�
	unsigned  uThrdID = 0;
	m_pThrdreco = (HANDLE)_beginthreadex( nullptr, 0, CMSSR::StartReco, this, 0, &uThrdID);
	m_bInited = true;
	Stop();
	return 0;
}

unsigned __stdcall CMSSR::StartReco(void* pParam)
{
	CMSSR* pThis = (CMSSR*)pParam;
	if (pThis == nullptr)
	{
		pThis->m_bthrd_reco_exit = true;
		return -1;
	}
	USES_CONVERSION;
	CComPtr<ISpRecoResult> cpResult;
	while (pThis->m_bInited)
	{
		cpResult.Release();
		if (FAILED(pThis->BlockWaitForReco(&cpResult)))
		{
			continue;
		}

		if (pThis->m_bLoadCmdGrammer)
		{
			pThis->m_cpCmdGrammar->SetRuleState(NULL, NULL, SPRS_INACTIVE);
		}
		else
		{
			pThis->m_cpCmdGrammar->SetDictationState(SPRS_INACTIVE);
		}
		
		CSpDynamicString dstrText;
		static const WCHAR wszUnrecognized[] = L"<Unrecognized>";
		if (FAILED(cpResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE,
			TRUE, &dstrText, NULL)))
		{
			dstrText = wszUnrecognized;
		}
		BSTR SRout;
		dstrText.CopyToBSTR(&SRout);
		DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, SRout, -1, NULL, 0, NULL, FALSE);
		char *psText;
		psText = new char[dwNum];
		WideCharToMultiByte(CP_OEMCP, NULL, SRout, -1, psText, dwNum, NULL, FALSE);
		pThis->DoRecoText(psText,pThis->m_bRecoRbtName);
		delete[]psText;
		if (pThis->m_bLoadCmdGrammer)
		{
			pThis->m_cpCmdGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);
		}
		else
		{
			pThis->m_cpCmdGrammar->SetDictationState(SPRS_ACTIVE);
		}
	}
	pThis->m_bthrd_reco_exit = true;
	return 0;
}

int CMSSR::SetMSSRCb(CallbackFun mssr_cb)
{
	m_mssr_cb = mssr_cb;
	return 0;
}

int CMSSR::SetMSSRSoundStart(CallbackFunSoundStart mssr_sound_start)
{
	m_mssr_sound_start = mssr_sound_start;
	return 0;
}

int CMSSR::SetMSSRSoundEnd(CallbackFunSoundStart mssr_sound_end)
{
	m_mssr_sound_end = mssr_sound_end;
	return 0;
}

int CMSSR::Start()
{
	//HRESULT hr = m_cpRecoEngine->SetRecoState(SPRST_ACTIVE);
	if (!m_bInited)
	{
		return -1;
	}
	/*HRESULT hr = m_cpRecoCtxt->SetContextState(SPCS_ENABLED);
	if (SUCCEEDED(hr))
	{
	speak("��ʼ����ʶ��");
	return 0;
	}
	else
	{
	return -1;
	}*/
	m_bRecord = true;
	return 0;
}

int CMSSR::Stop()
{
	//HRESULT hr = m_cpRecoEngine->SetRecoState(SPRST_INACTIVE);
	if (!m_bInited)
	{
		return -1;
	}
	/*HRESULT hr = m_cpRecoCtxt->SetContextState(SPCS_DISABLED);
	if (SUCCEEDED(hr))
	{
	return 0;
	}
	else
	{
	return -1;
	}*/
	m_bRecord = false;
	return 0;
}

HRESULT CMSSR::BlockWaitForReco(ISpRecoResult ** ppResult)
{
	HRESULT hr = S_OK;
	CSpEvent event;

	while (SUCCEEDED(hr) &&
		SUCCEEDED(hr = event.GetFrom(m_cpRecoCtxt)) &&
		hr == S_FALSE && m_bInited)
	{
		hr = m_cpRecoCtxt->WaitForNotifyEvent(100);
	}
	hr = E_FAIL;
	std::string str;
	switch (event.eEventId)
	{
	case SPEI_INTERFERENCE:
	{
		USES_CONVERSION;
		switch (event.Interference())
		{
		case SPINTERFERENCE_NONE:
			str = "���ţ���";
			break;
		case SPINTERFERENCE_NOISE:
			str = "���ţ�����";
			break;
		case SPINTERFERENCE_NOSIGNAL:
			str = "���ţ����ź�";
			break;
		case SPINTERFERENCE_TOOLOUD:
			str = "���ţ�����̫��";
			break;
		case SPINTERFERENCE_TOOQUIET:
			str = "���ţ�̫����";
			break;
		case SPINTERFERENCE_TOOFAST:
			str = "����ģʽ: ���ţ�̫��";
			break;
		case SPINTERFERENCE_TOOSLOW:
			str = "����ģʽ: ���ţ�̫��";
			break;
		default:
			str = "����ʶ��ĸ����¼�";
		}
	}
	break;
	case SPEI_RECO_STATE_CHANGE:
		break;
	case SPEI_SOUND_START:
		m_bInSound = TRUE;
		str = "������ʼ";
		m_mssr_sound_start();
		break;
	case SPEI_SOUND_END:
		if (m_bInSound)
		{
			m_bInSound = FALSE;
			if (!m_bGotReco)
			{
				str = "û��˵��������";
			}
			m_bGotReco = FALSE;
		}
		str = "��������";
		m_mssr_sound_end();
		////LOGINFO("��������");
		break;

	case SPEI_RECO_OTHER_CONTEXT:
	{
		m_bGotReco = TRUE;
		str = "�����ͻ��˵�ʶ����";
	}
	break;
	case SPEI_FALSE_RECOGNITION:
	case SPEI_HYPOTHESIS:
	case SPEI_RECOGNITION:
	{
		if (event.eEventId == SPEI_RECOGNITION && m_bRecord)
		{
			*ppResult = event.RecoResult();
			if (*ppResult)
			{
				(*ppResult)->AddRef();
			}
			hr = S_OK;
			str = "�ɹ�ʶ��";
			//LOGINFO("�ɹ�ʶ��");
		}else if (event.eEventId == SPEI_FALSE_RECOGNITION)
		{
			str = "û��ʶ��";
		}	
	}
	break;
	}
	return hr;
}

int CMSSR::Uninit()
{
	if (!m_bInited)
	{
		return -1;
	}
	Stop();
	m_bInited = false;
	
	while (!m_bthrd_reco_exit)
	{
		Sleep(200);
	}

	if ( nullptr != m_pThrdreco)
	{
		CloseHandle(m_pThrdreco);
		m_pThrdreco = nullptr;
	}
	::CoInitialize(NULL);
	return 0;
}

int CMSSR::EnumSRObject()
{
	/*CComPtr<ISpObjectTokenCategory> cpSpCategory;
	CComPtr<IEnumSpObjectTokens> cpSpEnumTokens;
	CComPtr<ISpObjectToken>  pISpObjectToken;
	HRESULT hr;
	hr = SpGetCategoryFromId(SPCAT_VOICES, &cpSpCategory);
	hr = cpSpCategory->EnumTokens(NULL, NULL, &cpSpEnumTokens);
	if (SUCCEEDED(hr))
	{
		ULONG ulTokensNumber = 0;
		cpSpEnumTokens->GetCount(&ulTokensNumber);

		if (ulTokensNumber == 0)
		{
			return -1;
		}

		CString strVoicePackageName = _T("");
		CString strTokenPrefixText = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\");
		for (ULONG i = 0; i < ulTokensNumber; i++)
		{
			cpSpEnumTokens->Item(i, &pISpObjectToken);
			WCHAR* pChar;
			pISpObjectToken->GetId(&pChar);
			strVoicePackageName = pChar;
			strVoicePackageName.Delete(0, strTokenPrefixText.GetLength());
			pISpObjectToken.Release();
		}
	}
	cpSpCategory.Release();
	cpSpEnumTokens.Release();
	hr = SpGetCategoryFromId(SPCAT_RECOGNIZERS, &cpSpCategory);
	hr = cpSpCategory->EnumTokens(NULL, NULL, &cpSpEnumTokens);
	if (SUCCEEDED(hr))
	{
		ULONG ulTokensNumber = 0;
		cpSpEnumTokens->GetCount(&ulTokensNumber);

		if (ulTokensNumber == 0)
		{
			return -1;
		}

		CString strVoicePackageName = _T("");
		CString strTokenPrefixText = _T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\");
		for (ULONG i = 0; i < ulTokensNumber; i++)
		{
			cpSpEnumTokens->Item(i, &pISpObjectToken);
			WCHAR* pChar;
			pISpObjectToken->GetId(&pChar);
			strVoicePackageName = pChar;
			strVoicePackageName.Delete(0, strTokenPrefixText.GetLength());
			pISpObjectToken.Release();
		}
	}*/

	return 0;
}

int CMSSR::DoRecoText(const char* str,bool bRecoRbtName)
{
	HRESULT hr = E_FAIL;
	if(str == nullptr)
		return hr;
	if (m_mssr_cb)
	{
		std::string sResult(str);

		// ȥ����������ʶ���еĿո�
		int index = 0;
		if( !sResult.empty())
		{
			while( (index = sResult.find(' ',index)) != string::npos)
			{
				sResult.erase(index,1);
			}
		}
		hr = m_mssr_cb(sResult.c_str(),bRecoRbtName);
	}
	else
	{
		WCHAR wStr[256] = L"";
		MultiByteToWideChar(CP_ACP, 0,str, -1, wStr, 256);
		hr = m_cpVoice->Speak(wStr, 0, NULL);
	}
	return hr;
}

int CMSSR::GetConfig()
{
	m_cmdXmlFilePath.clear();
	m_cmdXmlFilePath = GetModuleFilePath()+"\\config\\LocalSRCmd.xml";
	return 0;
}


int CMSSR::FormatCmdXMLFile(std::string sStr)
{
	// �������������Ϊ�գ�ֻʶ������������˼������ģʽ����ΪInActive
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError xmlErr = doc.LoadFile(m_cmdXmlFilePath.c_str());
	if ( xmlErr != 0 )
	{
		return -1;
	}
	tinyxml2::XMLElement *pConfig = nullptr;
	pConfig = doc.FirstChildElement("GRAMMAR");
	if ( nullptr == pConfig )
	{
		return -1;
	}
	// <DEFINE></DEFINE>
	tinyxml2::XMLElement *pDefine = pConfig->FirstChildElement("DEFINE");
	if (nullptr == pDefine)
	{
		return -1;
	}
	// <RULE></RULE>
	tinyxml2::XMLElement *pRuleRbtCtl = pConfig->FirstChildElement("RULE");
	if (nullptr == pRuleRbtCtl)
	{
		return -1;
	}
	std::string strTemp;
	// ����������Ϊ��ʱ������ʶ�� ����+���� ����������ʶ��
	
	// <RULE><P></P></RULE> �޸Ļ���������
	tinyxml2::XMLElement *pRobotO = pRuleRbtCtl->FirstChildElement("O");
	if (nullptr == pRobotO)
	{
		return -1;
	}
	strTemp = G2U(sStr.c_str());
	pRobotO->SetText(strTemp.c_str());	
			
	//// <RULE></RULE>
	//tinyxml2::XMLElement *pRuleNormal = pRuleRbtCtl->NextSiblingElement();
	//if (nullptr == pRuleNormal)
	//{
	//	return -1;
	//}
	//// <RULE><L></L></RULE>
	//tinyxml2::XMLElement *pRuleNormalL = pRuleNormal->FirstChildElement();
	//if (nullptr == pRuleNormalL)
	//{
	//	return -1;
	//}
	//// <RULE><L><P></P></L></RULE> �޸Ļ���������
	//tinyxml2::XMLElement *pRuleNormalLP = pRuleNormalL->FirstChildElement();
	//if (nullptr == pRuleNormalLP)
	//{
	//	return -1;
	//}
	//pRuleNormalLP->SetText(strTemp.c_str());
	// �����޸�
	if ( doc.SaveFile(m_cmdXmlFilePath.c_str()) != tinyxml2::XML_SUCCESS )
	{
		return -1;
	}
	return 0;

}

int CMSSR::GetCtrlCmd(const std::list<std::pair<std::string, int > >& lsVoiceCtrl,std::list<std::pair<std::string, int > >&lsRobotCtrl)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError xmlErr = doc.LoadFile(m_cmdXmlFilePath.c_str());
	if ( xmlErr != 0 )
	{
		return -1;
	}
	tinyxml2::XMLElement *pConfig = nullptr;
	pConfig = doc.FirstChildElement("GRAMMAR");
	if ( nullptr == pConfig )
	{
		return -1;
	}
	// <DEFINE></DEFINE>
	tinyxml2::XMLElement *pDefine = pConfig->FirstChildElement("DEFINE");
	if (nullptr == pDefine)
	{
		return -1;
	}
	// <RULE></RULE>
	tinyxml2::XMLElement *pRuleRbtCtl = pConfig->FirstChildElement("RULE");
	if (nullptr == pRuleRbtCtl)
	{
		return -1;
	}
	std::string strTemp;
	// ����������Ϊ��ʱ������ʶ�� ����+���� ����������ʶ��

	// <RULE><P></P></RULE> ��ȡ���������ֵ�ƴ��
	tinyxml2::XMLElement *pRobotO = pRuleRbtCtl->FirstChildElement("O");
	if (nullptr == pRobotO)
	{
		return -1;
	}
	std::string sRobotName = U2G(pRobotO->GetText());
	std::string sRobotPY;

	tinyxml2::XMLElement *pRobotOCmd = pRobotO->NextSiblingElement("O");
	if (nullptr == pRobotOCmd)
	{
		return -1;
	}
	// <O><LIST></LIST></O>
	tinyxml2::XMLElement *pRobotOCmdList = pRobotOCmd->FirstChildElement("LIST");
	if (nullptr == pRobotOCmdList)
	{
		return -1;
	}
	// <O><LIST><P></P></LIST></O>
	tinyxml2::XMLElement *pRobotOCmdListP = pRobotOCmdList->FirstChildElement("P");
	if (nullptr == pRobotOCmdListP)
	{
		return -1;
	}

	while(nullptr != pRobotOCmdListP)
	{
		std::string sPName;
		if(pRobotOCmdListP->Attribute("PROPNAME"))
		{
			sPName = pRobotOCmdListP->Attribute("PROPNAME");
		}
		auto iter = lsVoiceCtrl.begin();
		for ( iter; iter != lsVoiceCtrl.end(); ++iter )
		{
			if ( 0 == iter->first.compare(sPName) )
			{
				tinyxml2::XMLElement *pRobotOCmdListPList = pRobotOCmdListP->FirstChildElement("LIST");
				int sCtrlType = iter->second;
				if (nullptr != pRobotOCmdListPList)
				{
					tinyxml2::XMLElement *pRobotOCmdListPListP = pRobotOCmdListPList->FirstChildElement("P");
					while (nullptr != pRobotOCmdListPListP)
					{
						std::string sTemp = U2G(pRobotOCmdListPListP->GetText());
						if (!sTemp.empty())
						{
							lsRobotCtrl.push_back(std::pair<std::string, int> ( sRobotPY + sTemp, sCtrlType));
						}
						pRobotOCmdListPListP = pRobotOCmdListPListP->NextSiblingElement("P");
					}
				}

			}
		}
		pRobotOCmdListP = pRobotOCmdListP->NextSiblingElement("P");
	}
	return 0;
}

int CMSSR::GetRobotCtrl(SLocalCmd* pInParam,int nInLen,SLocalCmd* pOutParam,int nOutLen)
{
	GetConfig();
	if (nullptr == pInParam || nullptr == pOutParam)
	{
		return -1;
	}

	tinyxml2::XMLDocument doc;
	tinyxml2::XMLError xmlErr = doc.LoadFile(m_cmdXmlFilePath.c_str());
	if ( xmlErr != 0 )
	{
		return -1;
	}
	tinyxml2::XMLElement *pConfig = nullptr;
	pConfig = doc.FirstChildElement("GRAMMAR");
	if ( nullptr == pConfig )
	{
		return -1;
	}
	// <DEFINE></DEFINE>
	tinyxml2::XMLElement *pDefine = pConfig->FirstChildElement("DEFINE");
	if (nullptr == pDefine)
	{
		return -1;
	}
	// <RULE></RULE>
	tinyxml2::XMLElement *pRuleRbtCtl = pConfig->FirstChildElement("RULE");
	if (nullptr == pRuleRbtCtl)
	{
		return -1;
	}
	std::string strTemp;
	// ����������Ϊ��ʱ������ʶ�� ����+���� ����������ʶ��

	// <RULE><P></P></RULE> ��ȡ���������ֵ�ƴ��
	tinyxml2::XMLElement *pRobotO = pRuleRbtCtl->FirstChildElement("O");
	if (nullptr == pRobotO)
	{
		return -1;
	}
	std::string sRobotName = U2G(pRobotO->GetText());
	std::string sRobotPY;

	tinyxml2::XMLElement *pRobotOCmd = pRobotO->NextSiblingElement("O");
	if (nullptr == pRobotOCmd)
	{
		return -1;
	}
	// <O><LIST></LIST></O>
	tinyxml2::XMLElement *pRobotOCmdList = pRobotOCmd->FirstChildElement("LIST");
	if (nullptr == pRobotOCmdList)
	{
		return -1;
	}
	// <O><LIST><P></P></LIST></O>
	tinyxml2::XMLElement *pRobotOCmdListP = pRobotOCmdList->FirstChildElement("P");
	if (nullptr == pRobotOCmdListP)
	{
		return -1;
	}
	int nOutRealLen = 0;
	while(nullptr != pRobotOCmdListP)
	{
		std::string sPName;
		if(pRobotOCmdListP->Attribute("PROPNAME"))
		{
			sPName = pRobotOCmdListP->Attribute("PROPNAME");
		}
		
		for ( int i=0; i< nInLen; ++i )
		{
			if ( 0 == strcmp(sPName.c_str(),pInParam[i].str) )
			{
				tinyxml2::XMLElement *pRobotOCmdListPList = pRobotOCmdListP->FirstChildElement("LIST");
				int sCtrlType = pInParam[i].nType;
				if (nullptr != pRobotOCmdListPList)
				{
					tinyxml2::XMLElement *pRobotOCmdListPListP = pRobotOCmdListPList->FirstChildElement("P");
					while (nullptr != pRobotOCmdListPListP)
					{
						std::string sTemp = U2G(pRobotOCmdListPListP->GetText());
						if (!sTemp.empty())
						{
							//lsRobotCtrl.push_back(std::pair<std::string, int> ( sRobotPY + sTemp, sCtrlType));
							if (nOutRealLen < nOutLen)
							{
								strcpy(pOutParam[nOutRealLen].str,sTemp.c_str());
								pOutParam[nOutRealLen].nType = sCtrlType;
								nOutRealLen++;
							}else
							{
								break;
							}
							
						}
						pRobotOCmdListPListP = pRobotOCmdListPListP->NextSiblingElement("P");
					}
				}

			}
		}
		pRobotOCmdListP = pRobotOCmdListP->NextSiblingElement("P");
	}
	
	return 0;
}