#pragma once

#include "..\\..\\cdf\\zANT.h"

class subANT:public zANT{
public:
	subANT();
	HRESULT				OnNewData();
	LRESULT CALLBACK	m_MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	virtual void	variantSetup();			

public:

};

static subANT *sp;
