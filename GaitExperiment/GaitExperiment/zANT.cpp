#include "stdafx.h"
#include "zANT.h"


#include "metronome.h"
#include "isense.h"


static LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	if(zp!=NULL){
		return zp->m_MsgProc(hWnd, uMsg, wParam, lParam);
	}
	else{
		printf("A relevant object has not been created\n");
		return -1;
	}
}

//-----------------------------------------------------------------------------
// Name: zANT()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
zANT::zANT()
{	
	printf("Base Class setup\n");
	invariantSetup();
	//don't register the message window here, do it in the derived class

	//registerMessageWindow();
	variantSetup();
}
void zANT::invariantSetup(){
	
	// Enable run-time memory check for debug builds.
	_CrtSetDbgFlag(
		_CRTDBG_ALLOC_MEM_DF |
		_CRTDBG_LEAK_CHECK_DF |
		_CRTDBG_CHECK_ALWAYS_DF |
		_CRTDBG_CHECK_EVERY_16_DF);
	
	// initialize variables
	m_nSampleIndex		= 0;
	m_nSampleCount		= (ULONG)(DEFAULT_WINDOW * (double)DEFAULT_SAMPLINGFREQ);
	m_nChannelCount		= DEFAULT_CHANNELCOUNT;

	ZeroMemory(m_dChannelOffset, sizeof(m_dChannelOffset));
	ZeroMemory(m_bKey, sizeof(m_bKey));

	//******************* set up flags *********************
	m_useTestData = false;
	m_verbose = false;
	m_saving = false;

	//set up variables
	consoleInput = (char *) malloc(sizeof(char) * DEFAULT_CONSOLE_INPUT_SIZE);
	consoleInputIdx = 0;
	m_filePartIdx = 0;	
	
	m_nchan = &(m_oAcquisition.m_nchan);
	m_nsamp = &(m_oAcquisition.m_nsamp);
	setupTestData();
	testDataOffset = 0;	

	
}
void zANT::registerMessageWindow(){
	//register window messages
	MSG_ONNEWDATA		= RegisterWindowMessage(_T("MSG_ONNEWDATA"));
	MSG_ONOVERFLOW		= RegisterWindowMessage(_T("MSG_ONOVERFLOW"));
	MSG_ONBUFFERERROR	= RegisterWindowMessage(_T("MSG_ONBUFFERERROR"));
	MSG_ONSPLITDATA		= RegisterWindowMessage(_T("MSG_ONSPLITDATA"));
		


	// *************************** Create Message-Only Window ************************************
	HINSTANCE hinst;
    WNDCLASSEX wnd;    

	hinst = GetModuleHandle( NULL );
    memset( &wnd, 0, sizeof( wnd ) );
    wnd.cbSize = sizeof( wnd );
    wnd.lpszClassName = L"MainWClass";
    wnd.lpfnWndProc = MsgProc; //TODO CHECK TO MAKE SURE THIS DOESN'T HAVE TO GO IN VARIANT maybe can go in invariant, too tired to think
    wnd.hInstance = hinst;
	int result = RegisterClassEx( &wnd );
    if( !result )
    {
        printf("RegisterClassEx error: %d\r\n", GetLastError() );
    }
	m_hWnd = CreateWindowEx
        (
        0, //extended styles
        wnd.lpszClassName, //class name
        L"Main Window", //window name
        WS_MINIMIZEBOX, //style tags
        0, //horizontal position
        0, //vertical position
        0, //width
        0, //height
        HWND_MESSAGE, //parent window
        (HMENU) NULL, //class menu
        (HINSTANCE) wnd.hInstance, //some HINSTANCE pointer
        NULL //Create Window Data?
        );	
	if(m_hWnd==NULL)
		printf("Error creating window: 0x%x\n",GetLastError());
	// *************************** End Create Message-Only Window ********************************
}
void zANT::variantSetup(){
	if(zp!=NULL){
		printf("Only one instance of zANT can be active at a time (it's a kludge, sorry!)\n");
		free(consoleInput);
		throw kludgeError;
	}
		
	zp = this; //kludge to get the message only window to work and affect the (ONLY ONE!!) instance of this object that should exist			
}

zANT::~zANT()
{
	cleanup();
}

void zANT::cleanup(){	
	if(cdfs.isOpen()){	
		closeFile();
	}	
	free(consoleInput);	
	DestroyWindow(m_hWnd);	
}

void zANT::setFileName(char f[]){
	memset(m_filename,NULL,DEFAULT_FILENAME_SIZE);
	strcpy(m_filename,f);	
}

bool zANT::startNewFile(){
	m_saving = false;
	if(cdfs.isOpen()){
		closeFile();
	}
	char filename[DEFAULT_FILENAME_SIZE+2];	
	if(m_filename!=NULL)
		sprintf(filename,"%s%02d.cdf",m_filename, m_filePartIdx);	
	else
		sprintf(filename,"part%02d.cdf",m_filePartIdx);
	cdfs.open(filename,true);	
	cdf::getVariableHeaderSize(cdfs.session);
	sprintf(m_msg,"starting new file \"%s\", variable header size %d",filename,cdfs.session.variableHeaderSize);
	verboseMessage(m_msg);	
	m_nframe = 0;	
	m_fileOpen = true;
	return cdfs.isGood();
}

bool zANT::closeFile(){	
	m_saving = false;
	cdfs.writeHeader();
	cdfs.close();
	m_fileOpen = false;
	sprintf(m_msg,"wrote %d frames",m_nframe);
	verboseMessage();
	m_filePartIdx++;
	return true;
}

void zANT::verboseMessage(char *m){
	if(m_verbose)
		printf("%s\n",m);
}

HRESULT zANT::InitAcquisitionObject(){
	HRESULT hr;
	printf("initializing acquisition object...\n");
	// initialize the acquisition object
	if (m_oAcquisition == NULL)
	{
		// initialize the acquisition device
		hr = m_oAcquisition.Initialize();		
		if FAILED(hr)
			return hr;
		// try to connect to acquisition device
		VARIANT_BOOL bConnected;
		bConnected = m_oAcquisition->connect(_T(DEFAULT_DEVICE), DEFAULT_SAMPLINGFREQ, _T(DEFAULT_LIBRARY));
		if (!bConnected)
		{
			printf("----------------------ATTENTION: AMPLIFIER DISCONNECTED---------------------\n");
			printf("Failed to connect to acquisition device\nTrying to connect to test device...");
			
			// try to connect to TEST device
			TCHAR szLibraryPath[MAX_PATH];
			GetModuleFileName(NULL, szLibraryPath, MAX_PATH);
			PathRemoveFileSpec(szLibraryPath);
			PathAddBackslash(szLibraryPath);
			PathAppend(szLibraryPath, _T(DEFAULT_LIBRARY_TEST));

			bConnected = m_oAcquisition->connect(_T(DEFAULT_DEVICE), DEFAULT_SAMPLINGFREQ, szLibraryPath);
			if (!bConnected)
			{
				// try to connect to alternative TEST device
				TCHAR szLibraryPath[MAX_PATH];
				GetModuleFileName(NULL, szLibraryPath, MAX_PATH);
				PathRemoveFileSpec(szLibraryPath);
				PathAddBackslash(szLibraryPath);
				PathAppend(szLibraryPath, _T(DEFAULT_LIBRARY_TEST_ALT));
	
				bConnected = m_oAcquisition->connect(_T(DEFAULT_DEVICE), DEFAULT_SAMPLINGFREQ, szLibraryPath);
				if (!bConnected) {
					printf("\nFailed to connect to acquisition test device\naborted\n");
					return S_FALSE;
				}
			}
			printf("success\n");
			printf("----------------------------------------------------------------------------\n");
		}

		// set mode
		m_oAcquisition->mode = _T("lowpower");
		//ATLASSERT((ULONG)m_oAcquisition->mode == 0); 

		// set updating interval
		m_oAcquisition->bufferInterval = 0.033; // about 30 FPS

		m_oAcquisition->samplingRate = 512;
		// override channel count
		//m_nChannelCount = m_oAcquisition->channelCount;

		// start acquisition device
		m_oAcquisition->state = _T("active");
		CComVariant vState = m_oAcquisition->Getstate();
		if (vState.ulVal != 1)
			return S_FALSE;

		m_oAcquisition.AttachWindow(m_hWnd);		
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DeleteAcquisitionObject()
// Desc: Delete the acquisition object.
//-----------------------------------------------------------------------------
HRESULT zANT::DeleteAcquisitionObject()
{
	HRESULT hr;

	// detach parent window (accept no messages)
	m_oAcquisition.DetachWindow();

	// cleanup the acquisition object
	if (m_oAcquisition != NULL)
	{
		// stop acquisition device
		m_oAcquisition->state = _T("idle");
		CComVariant vState = m_oAcquisition->Getstate();

		ATLASSERT(vState.ulVal == 0 || vState.vt == VT_BSTR);

		// disconnect acquisition device
		VARIANT_BOOL bDisconnected;
		bDisconnected = m_oAcquisition->disconnect();
		ATLASSERT(bDisconnected);

		// clean-up
		hr = m_oAcquisition.Cleanup();
		ATLASSERT(SUCCEEDED(hr));
	}
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: getConsoleInput()
// Desc: gets cosnole input and adds it to the recent console input
//-----------------------------------------------------------------------------
char zANT::getConsoleInput(){
	char c=0;
	if(_kbhit()){
		c = _getch();				
	}	
	int i;
	if(consoleInputIdx<DEFAULT_CONSOLE_INPUT_SIZE){
		consoleInput[consoleInputIdx] = c;
		consoleInputIdx++;
	}
	else{
		for(i=0;i<DEFAULT_CONSOLE_INPUT_SIZE-1;i++)
		consoleInput[i] = consoleInput[i+1];
		consoleInput[DEFAULT_CONSOLE_INPUT_SIZE-1] = c;
	}	
	
	return c;
}

BYTE* zANT::getbKeyPtr(unsigned int idx){
	return m_bKey+idx;
}
CAcquisition* zANT::getAcqPtr(){
	return &m_oAcquisition;
}
UINT zANT::getMSG_ONNEWDATA(){
	return MSG_ONNEWDATA;
}
UINT zANT::getMSG_ONOVERFLOW(){
	return MSG_ONOVERFLOW;
}
UINT zANT::getMSG_ONBUFFERERROR(){
	return MSG_ONBUFFERERROR;
}
UINT zANT::getMSG_ONSPLITDATA(){
	return MSG_ONSPLITDATA;
}
UINT zANT::numChan(){
	return m_oAcquisition.m_nchan;	
}
UINT zANT::numSamp(){
	return m_oAcquisition.m_nsamp;		
}


void zANT::toggleSaving(){
	char *onoff;
	m_saving = !m_saving;
	(m_saving)? onoff="Started":onoff="Stopped";
	printf("%s saving\n",onoff);
}

void zANT::toggleTestData(){
	char *onoff;
	m_useTestData = !m_useTestData;			
	(m_useTestData)? onoff="Now":onoff="Not";
	printf("%s using test data\n",onoff);
}

void zANT::toggleVerbose(){
	char *onoff;
	m_verbose = !m_verbose;
	(m_verbose)? onoff="on":onoff="off";
	printf("Verbosity is %s\n",onoff);
}

//-----------------------------------------------------------------------------
// Name: OnNewData()
// Desc: Called when the acquisition device received new data
//-----------------------------------------------------------------------------
HRESULT zANT::OnNewData()
{	
	//printf("OnNewData() in zANT\n");
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: hWnd()
// Desc: returns window handle
//-----------------------------------------------------------------------------
HWND zANT::hWnd(){
	return m_hWnd;
}

void zANT::setupTestData(){
	//TODO: Find out the order of samples, is it chan x samp or samp x chan?
	int i;
	int s;		
	for(s=0;s<512;s++){
		for(i=0;i<DEFAULT_NCHANNELS;i++){
			testData[s*DEFAULT_NCHANNELS+i] = cos(float(2.0*3.14159267*i*s/512.0));			
		}
	}	
}

int zANT::handleConsoleInput(){
	char c = getConsoleInput();
	if(c)
		PostMessage(hWnd(),WM_KEYDOWN,c,0);	
	return 0;
}

//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Process windows messages.
//-----------------------------------------------------------------------------
LRESULT CALLBACK zANT::m_MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{		
	
	//printf("uMsg = 0x%x\n",uMsg);
	if(zp==NULL){		
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	
	switch(uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_KEYUP:
		*(zp->getbKeyPtr(wParam)) = FALSE;
		break;

	case WM_KEYDOWN:
		*(zp->getbKeyPtr(wParam)) = TRUE;
		
		if(wParam == 'n' || wParam == 'N'){
			if(!zp->startNewFile()){
				printf("could not open new file, aborting\n");
				throw(cdf::fileReadError);
			}						
		}		
		else if(wParam == 'q' || wParam == 'Q'){
			PostQuitMessage(0);
			return 0;
		}
		else if(wParam == 's' || wParam == 'S'){
			zp->toggleSaving();
			
		}
		else if(wParam == 't' || wParam == 'T'){
			zp->toggleTestData();
		}

		else if(wParam == 'v' || wParam == 'V'){
			zp->toggleVerbose();
		}
		else
			printf("nothing mapped for keystroke \'%c\'\n", wParam);
		break;
	}

	if (uMsg == zp->getMSG_ONNEWDATA())
		return zp->OnNewData();

		
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

