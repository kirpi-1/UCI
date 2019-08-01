#pragma once
#include "stdafx.h"
#include "zANT.h"

#include "icube.h"

class gExperiment:public zANT{
public:
	gExperiment();
	HRESULT			OnNewData();
	LRESULT CALLBACK m_MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	virtual void	variantSetup();		
	bool			m_useCubes;
	bool			m_displayCubes;
	

public:
	bool useCubes(){return m_useCubes;};
	void toggleCubes(){toggleCubes(!m_useCubes);};
	void toggleCubes(bool u);
	void toggleDisplayCubes();
};

static gExperiment *gp;