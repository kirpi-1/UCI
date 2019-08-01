//#include "stdafx.h"
//#include "udpServer.h"
//
//
//SOCKET setupUDPServer(char *port){
//	SOCKET s;	
//	struct addrinfo hints, *servinfo, *p;
//	memset(&hints, 0, sizeof(hints));
//	hints.ai_family = AF_INET;
//	hints.ai_socktype = SOCK_DGRAM;
//	hints.ai_flags = AI_PASSIVE;
//
//	getaddrinfo(NULL, port,&hints,&servinfo);
//	int ret;
//	for(p=servinfo;p!=NULL;p=p->ai_next){
//		s = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
//		if(s==-1)
//			continue;
//		ret = bind(s,p->ai_addr,p->ai_addrlen);
//		if(ret == -1){
//			closesocket(s);
//			continue;
//		}
//		break;
//	}
//
//	if(p == NULL){
//		DWORD err = WSAGetLastError();
//		WSACleanup();
//		return -1;
//	}
//	freeaddrinfo(servinfo);
//
//	u_long iMode=1; //iMode = 1 for non-blocking socket
//	ioctlsocket(s,FIONBIO,&iMode);
//	
//	return s;
//}