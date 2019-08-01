// MyCAsyncSocket.cpp : implementation file
//

#include "stdafx.h"
#include "ExperimentControl.h"
#include "MyCAsyncSocket.h"


// MyCAsyncSocket

MyCAsyncSocket::MyCAsyncSocket()
{
	
	this->CAsyncSocket::CAsyncSocket();
}
MyCAsyncSocket::MyCAsyncSocket(HANDLE h)
{
	setConnectedEvent(h);
	AfxMessageBox(_T("set event!"));
}
/*
MyCAsyncSocket::~MyCAsyncSocket()
{
}

*/
// MyCAsyncSocket member functions

void MyCAsyncSocket::OnConnect(int nErrorCode){
	AfxMessageBox(_T("OnConnect!"));
	SetEvent(m_connected);	
	CAsyncSocket::OnConnect(nErrorCode);
}

void MyCAsyncSocket::OnClose(int nErrorCode){
	AfxMessageBox(_T("OnClose!"));
	ResetEvent(m_connected);
	CAsyncSocket::OnClose(nErrorCode);
}