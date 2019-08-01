#pragma once
#include "atlbase.h"
#include "atlcom.h"
#include "atlsafe.h"

#import "C:\\Program Files\\Neurofeedback\\lib\\PeripheralsLib.tlb" no_namespace named_guids


#define	DISPID_ACQ_ONNEWDATA		2000
#define	DISPID_ACQ_ONOVERFLOW		2001
#define	DISPID_ACQ_ONBUFFERERROR	2002
#define DEFAULT_MBUFFER_SIZE		sizeof(float)*256*128

//---------------------------------------------------------------------------
// CAcquisition class
//---------------------------------------------------------------------------
class CAcquisition : 
	public IDispEventImpl<0, CAcquisition, &__uuidof(_IAcquisitionEvents), &LIBID_PeripheralsLib, 1, 0>
{
	friend class zANT;	
public:

	BEGIN_SINK_MAP(CAcquisition)
		SINK_ENTRY_EX(0, __uuidof(_IAcquisitionEvents), DISPID_ACQ_ONNEWDATA, OnNewDataHandler)
		SINK_ENTRY_EX(0, __uuidof(_IAcquisitionEvents), DISPID_ACQ_ONOVERFLOW, OnOverflowHandler)
		SINK_ENTRY_EX(0, __uuidof(_IAcquisitionEvents), DISPID_ACQ_ONBUFFERERROR, OnBufferErrorHandler)
	END_SINK_MAP()

	CAcquisition();
	virtual ~CAcquisition();

	HRESULT Initialize();
	HRESULT Cleanup();

	HRESULT AttachWindow(HWND hWndParent);
	HRESULT DetachWindow();

	operator IAcquisition*()		{ return m_spDispatch; }
	IAcquisition* operator ->()		{ return m_spDispatch; }

private:
	void __stdcall OnNewDataHandler();
	void __stdcall OnOverflowHandler();
	void __stdcall OnBufferErrorHandler();

protected:
	CComPtr<IAcquisition>	m_spDispatch;
	BOOL	m_initialized;
	float   *m_buffer;
	UINT	m_nsamp;
	UINT	m_nchan;
	
	
	HWND	m_hWndParent;
	
	UINT	MSG_ONNEWDATA;
	UINT	MSG_ONOVERFLOW;
	UINT	MSG_ONBUFFERERROR;
};
