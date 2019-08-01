#pragma once

// MyCAsyncSocket command target

class MyCAsyncSocket : public CAsyncSocket
{
public:
	MyCAsyncSocket();
	MyCAsyncSocket(HANDLE h);
	void setConnectedEvent(HANDLE h){m_connected = h;};
	HANDLE m_connected;	
protected:
	void OnConnect(int nErrorCode);
	void OnClose(int nErrorCode);
};


