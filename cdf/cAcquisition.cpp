#include "StdAfx.h"
#include "cAcquisition.h"


CAcquisition::CAcquisition()
{
	// Enable run-time memory check for debug builds.
	_CrtSetDbgFlag(
		_CRTDBG_ALLOC_MEM_DF |
		_CRTDBG_LEAK_CHECK_DF |
		_CRTDBG_CHECK_ALWAYS_DF |
		_CRTDBG_CHECK_EVERY_16_DF);

	m_spDispatch		= NULL;
	m_hWndParent		= NULL;

	MSG_ONNEWDATA		= RegisterWindowMessage(_T("MSG_ONNEWDATA"));
	MSG_ONOVERFLOW		= RegisterWindowMessage(_T("MSG_ONOVERFLOW"));
	MSG_ONBUFFERERROR	= RegisterWindowMessage(_T("MSG_ONBUFFERERROR"));
}

//---------------------------------------------------------------------------

CAcquisition::~CAcquisition()
{
	Cleanup();
}

//---------------------------------------------------------------------------

HRESULT CAcquisition::Initialize()
{
	HRESULT hr;

	// Instantiate the IAcquisition object
	ATLASSERT(m_spDispatch == 0);
	if FAILED(hr = m_spDispatch.CoCreateInstance(CLSID_Acquisition))
		return hr;

	m_buffer = (float*) malloc (DEFAULT_MBUFFER_SIZE);

	if (m_spDispatch->licensed.vt != VT_BOOL ||
		m_spDispatch->licensed.boolVal == VARIANT_FALSE)
	{
		printf("Acquisition control not licenced for use (0x%x)\n", MB_OK|MB_ICONERROR);
		m_spDispatch.Release();
		m_spDispatch = NULL;
		return CLASS_E_NOTLICENSED;
	}
	
	// Attach to event source
	if FAILED(hr = DispEventAdvise(*this))
		return hr;

	return S_OK;
}

//---------------------------------------------------------------------------

HRESULT CAcquisition::Cleanup()
{
	HRESULT hr = S_OK;

	// Disconnecting from the server's outgoing interface

	if (m_dwEventCookie != 0xFEFEFEFE)
		hr |= DispEventUnadvise(*this);


	if (m_spDispatch)
	{
		m_spDispatch.Release();
		m_spDispatch = NULL;
	}
	
	free(m_buffer);
	return hr;
}

//---------------------------------------------------------------------------

HRESULT CAcquisition::AttachWindow(HWND hWndParent)
{
	ATLASSERT(hWndParent != NULL);
	if (!hWndParent)
		return E_FAIL;

	m_hWndParent = hWndParent;
	return S_OK;
}

//---------------------------------------------------------------------------

HRESULT CAcquisition::DetachWindow()
{
	m_hWndParent = NULL;
	return S_OK;
}

//---------------------------------------------------------------------------

void __stdcall CAcquisition::OnNewDataHandler()
{
	//printf("OnNewDataHandler() in cAcquisition.cpp\n");
	ULONG c,s;
	LONG al[2];
	double dValue;
	
	if (m_buffer==(float *)0) return;
	//printf("checked m_buffer\n");
	CComVariant vMatrix;
	vMatrix = m_spDispatch->getData(CComVariant(0), VARIANT_FALSE);
	if (vMatrix.vt == VT_EMPTY){
		printf("vMatrix.vt is empty!\n");
		return;
	}
	//printf("going to assert\n");
	ATLASSERT((vMatrix.vt & VT_ARRAY) > 0);
	ATLASSERT((vMatrix.vt & VT_R8) > 0);

	// attach to the safearray
	CComSafeArray<double> saMatrix;
	saMatrix.Attach(vMatrix.parray);
	// get matrix dimensions
	ULONG nSampleCount = saMatrix.GetCount(0);
	ULONG nChannelCount = saMatrix.GetCount(1);
	m_nchan = nChannelCount;
	m_nsamp = nSampleCount;
	//printf("nChannelCount:\t%d\tnSampleCount:\t%d\n",nChannelCount, nSampleCount);
	if(nChannelCount*nSampleCount>=DEFAULT_MBUFFER_SIZE)
		printf("number of samples and channels is too big! %d > %d\n",m_nchan*m_nsamp, DEFAULT_MBUFFER_SIZE);
	//printf("checking if matrix is empty\n");
	// check if matrix is empty
	if (nSampleCount == 0 || nChannelCount == 0) 
	{
		saMatrix.Detach();
		printf("matrix is empty!\n");
		return;
	}

	// copy into m_buffer; matrix position [iSample, iChan]
	//printf("copying data (%d by %d) into m_buffer...", nChannelCount, nSampleCount);
	int maxSamples = DEFAULT_MBUFFER_SIZE/nChannelCount;
	int sStart = 0;
	if(nSampleCount>maxSamples){
		printf("too many samples, readjusting(%d > %d)...\n",nSampleCount,maxSamples);
		sStart = nSampleCount-maxSamples;
		m_nsamp = nSampleCount - sStart;
	}

	for (c=0;c<nChannelCount;c++)
	{
		//printf("\nchannel(%d)\n",c);
		al[1]=c;
		for (s=sStart;s<nSampleCount;s++)
		{
			//printf("\n\tsample(%d)",s);
			al[0]=s;
			saMatrix.MultiDimGetAt(al, dValue);
			//printf("first step, got %f\n", dValue);
			//printf("s: %d\tnSampleCount: %d\n",s,nSampleCount);
			if(c*nSampleCount+(s-sStart)>=DEFAULT_MBUFFER_SIZE){
				printf("%d TOO BIG IT'S GONNA BLOW!\N",c*nSampleCount+(s-sStart));
			}
			*(m_buffer+c*nSampleCount+(s-sStart))=(float)dValue;
			//printf("\t got it");
		}
	}
	//printf("\ndone.\n");

	saMatrix.Detach();

	//(*m_nframe)++;
	//printf("about to send message to parent %d in OnNewDataHandler in cAcquisition\n",m_hWndParent);
	if (m_hWndParent && MSG_ONNEWDATA)
		SendMessage(m_hWndParent, MSG_ONNEWDATA, 0, 0);
}

//---------------------------------------------------------------------------

void __stdcall CAcquisition::OnOverflowHandler()
{
	if (m_hWndParent && MSG_ONOVERFLOW)
		SendMessage(m_hWndParent, MSG_ONOVERFLOW, 0, 0);
}

//---------------------------------------------------------------------------

void __stdcall CAcquisition::OnBufferErrorHandler()
{
	if (m_hWndParent && MSG_ONBUFFERERROR)
		SendMessage(m_hWndParent, MSG_ONBUFFERERROR, 0, 0);
}

