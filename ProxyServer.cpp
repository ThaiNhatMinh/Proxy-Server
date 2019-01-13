// ProxyServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "Socket.h"
#include "helper.h"
#include "util.h"
#if defined(WIN32)
#include <WinSock2.h>
#endif
#include "ServicePools.h"
#define PORT 8888


int main(int argc, char** argv)
{
	http::InitSocket();
	ServicePool pool(100);

	int yes = 1; 


	Socket server;
	if(!server.Listen(PORT))
		exit(-1);

	
	// lose the pesky "address already in use" error message
	if (server.SetOpt(SOL_SOCKET, SO_REUSEADDR, (char*)&yes,sizeof(int)) == -1) 
	{
		perror("setsockopt");
		exit(1);
	}

	// main loop
	for (;;) 
	{
		struct sockaddr_in remoteaddr;
		Socket cl = server.Accept(&remoteaddr);
		if (cl.IsValid())
		{
			if (!pool.AddSerivce(std::move(cl)))
			{
				std::cout << "Can't add socket to pool, pool is full\n";
			}
		}
	}



	http::DestroySocket();
}

