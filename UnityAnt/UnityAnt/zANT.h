#pragma once

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

#define DEFAULT_CONSOLE_INPUT_SIZE	64
#define DEFAULT_FILENAME_SIZE		64
#define MSG_SIZE					128
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

	//various flags
	bool			m_useTestData;
	bool			m_useNullData;
	bool			m_useAmpData;
	bool			m_verbose;	
	bool			m_saving;
	bool			m_fileOpen;
	
	void			setupTestData();
	float			testData[DEFAULT_NCHANNELS*512];
	int				testDataOffset;

	//for console messages
	char			m_msg[MSG_SIZE];
	
	UINT			m_nframe;
	char			m_filename[DEFAULT_FILENAME_SIZE];
	UINT			m_filePartIdx;
	

	CAcquisition	m_oAcquisition;
	BYTE			m_bKey[256];		

	ULONG			m_nSampleIndex;
	ULONG			m_nSampleCount;
	ULONG			m_nChannelCount;

	double			m_dChannelOffset[MAXIMUM_CHANNELS];

	char			*consoleInput;
	unsigned int	consoleInputIdx;		

	UINT	MSG_ONNEWDATA;
	UINT	MSG_ONOVERFLOW;
	UINT	MSG_ONBUFFERERROR;
	UINT	MSG_ONSPLITDATA;

public:
	HWND			m_hWnd; // handle to hidden application window
	float			*m_buffer; // numchannel by numsample (m-by-n array)
	UINT			*m_nsamp;
	UINT			*m_nchan;	
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
	UINT			numChan();
	UINT			numSamp();
	HWND			hWnd();
	virtual bool	startNewFile();
	bool			closeFile();

	HRESULT InitAcquisitionObject();
	HRESULT DeleteAcquisitionObject();
	
	char			getConsoleInput();
	int				handleConsoleInput();
	
	HRESULT FinalCleanup();
	
	CComVariant	getDataWrapper(CComVariant, VARIANT_BOOL);	

	virtual HRESULT OnNewData();
	HRESULT OnOverflow();
	HRESULT OnBufferError();

	void setFileName(char f[]);
	void setSubNum(int s){cdfs.session.subjectNumber = s;};

	void			verboseMessage(char *msg);
	void			verboseMessage(){verboseMessage(m_msg);};

	//functions for handling flags
	bool isFileOpen(){return m_fileOpen;};
	bool saving(){return m_saving;};	
	void toggleVerbose();
	void toggleSaving();
	void toggleSaving(bool s){m_saving = s;};	
	void toggleTestData();
	void toggleTestData(bool t);
	int  getDataType();
	void toggleNullData();
	void toggleNullData(bool n);
	void toggleAmpData();
	void toggleAmpData(bool a);
	bool verbose(){return m_verbose;};
};

static zANT *zp = NULL;
static LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);