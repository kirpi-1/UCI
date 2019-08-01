#include "subANT.h"


subANT::subANT(){
	printf("subANT setup...");
	invariantSetup();
	registerMessageWindow();
	variantSetup();
	printf("finished\n");
}

void subANT::variantSetup(){
	if(zp!=NULL){
		printf("you possibly just lost your last zp object...\n");		
	}
	zp = this;
	sp = this;
}

//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Process windows messages.
//-----------------------------------------------------------------------------
LRESULT CALLBACK subANT::m_MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{			
	int numbp = 0;
	//printf("subANT uMsg = 0x%x\n",uMsg);
	if(sp==NULL){		
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	
	switch(uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_KEYUP:
		*(sp->getbKeyPtr(wParam)) = FALSE;
		break;

	case WM_KEYDOWN:
		*(sp->getbKeyPtr(wParam)) = TRUE;
		//printf("subANT WM_KEYDOWN = 0x%x\n",wParam);
		if(wParam == 'q' || wParam == 'Q'){
			PostQuitMessage(0);
		}
		else if(wParam == 's' || wParam == 'S'){
			//save
		}
		else if(wParam == 'n' || wParam == 'N'){
			//new file
		}
		else
			zANT::m_MsgProc(hWnd,uMsg,wParam,lParam);				
		break;
	}

	if (uMsg == sp->getMSG_ONNEWDATA())
		return sp->OnNewData();
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HRESULT subANT::OnNewData()
{
	
	//get new data		
	cdfs.setFrameData(getBuffer(),getNumSamp(),getNumChan());
	
	return S_OK;
}