#pragma once
#pragma comment(lib,"comctl32.lib")
#include "stdafx.h"
#include "cAcquisition.h"
#include <conio.h>
#include "cdf.h"


#define	MAXIMUM_CHANNELS			128

#define DEFAULT_DEVICE				"tmsi"
#define DEFAULT_LIBRARY				""
#define DEFAULT_LIBRARY_TEST		"TMSITestDevice.dll"
#define DEFAULT_LIBRARY_TEST_ALT	"C:\\Program Files\\Neurofeedback\\lib\\TMSITestDevice.dll"
#define DEFAULT_SAMPLINGFREQ		512
#define DEFAULT_WINDOW				5.0
#define	DEFAULT_CHANNELCOUNT		8
#define	DEFAULT_USE_ONNEWDATA		TRUE 
#define DEFAULT_SCALE_Y				1e3

#define DEFAULT_NSAMPLES			32
#define DEFAULT_NCHANNELS			64
/*
#define DEFAULT_COLOR_BACKGROUND	0x2C63A2
#define DEFAULT_COLOR_EEGDATA		0xFFFFFF
#define DEFAULT_COLOR_EEGMARKER		0xFFFFFF
*/

#define DEFAULT_CONSOLE_INPUT_SIZE	32
#define DEFAULT_FILENAME_SIZE		32
struct SVertexEEG
{
    FLOAT    x, y, z;
};



class KludgeError{};
static KludgeError kludgeError;
struct MySockType{
	SOCKET clientSocket;
	SOCKET servSocket;
	HWND hWnd;
};

//-----------------------------------------------------------------------------
// Name: class zANT

//-----------------------------------------------------------------------------
class zANT //: public cWnd
{
public:
	zANT();
	~zANT();	
	virtual LRESULT CALLBACK m_MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	void			cleanup();
	virtual void	invariantSetup();
	void			registerMessageWindow();
	virtual void	variantSetup();
	void			verboseMessage(char *msg);
	void			verboseMessage(){verboseMessage(m_msg);};
	//various flags
	bool			m_useTestData;
	bool			m_verbose;
	char			m_msg[256];
		
	void			setupTestData();
	float			testData[DEFAULT_NCHANNELS*512];
	int				testDataOffset;

	UINT			m_nframe;
	char			m_filename[DEFAULT_FILENAME_SIZE];
	UINT			m_filePartIdx;
	

	CAcquisition	m_oAcquisition;
	BYTE			m_bKey[256];		

	ULONG			m_nSampleIndex;
	ULONG			m_nSampleCount;
	ULONG			m_nChannelCount;

	double			m_dChannelOffset[MAXIMUM_CHANNELS];	

	UINT			MSG_ONNEWDATA;
	UINT			MSG_ONOVERFLOW;
	UINT			MSG_ONBUFFERERROR;
	UINT			MSG_ONSPLITDATA;

	UINT			m_samplingRate;
	float			m_bufferInterval;
	void			setBufferInterval(float b);
	void			setSamplingRate(UINT s);
public:
	HWND			m_hWnd; // handle to hidden application window
	float*			getBuffer(){return m_oAcquisition.m_buffer;};
	UINT			getNumSamp(){return m_oAcquisition.m_nsamp;};
	UINT			getNumChan(){return m_oAcquisition.m_nchan;};

	MySockType		socket;

	HANDLE			childThread;
	
	//CDF for saving data
	cdf::CDFSession	cdfs;	


	BYTE*			getbKeyPtr(unsigned int idx);
	CAcquisition*	getAcqPtr();
	UINT			getMSG_ONNEWDATA();
	UINT			getMSG_ONOVERFLOW();
	UINT			getMSG_ONBUFFERERROR();
	UINT			getMSG_ONSPLITDATA();
	HWND			hWnd();

	HRESULT			InitAcquisitionObject();
	HRESULT			DeleteAcquisitionObject();
	
	char			getConsoleInput();
	int				handleConsoleInput();

	HRESULT FinalCleanup();
	
	CComVariant	getDataWrapper(CComVariant, VARIANT_BOOL);	

	virtual HRESULT OnNewData();
	HRESULT OnOverflow();
	HRESULT OnBufferError();
	

	//functions for handling flags
	void toggleVerbose();
	void toggleSaving();

};

static zANT *zp = NULL;
static LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);