// DllMssr.h : DllMssr DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CDllMssrApp
// �йش���ʵ�ֵ���Ϣ������� DllMssr.cpp
//

class CDllMssrApp : public CWinApp
{
public:
	CDllMssrApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
