// ProxyServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <cmath>
#include "Socket.h"
#include "helper.h"
#include "util.h"
#if defined(WIN32)
#include <WinSock2.h>
#endif
#include "ServicePools.h"
#define PORT 8888

void printusage()
{
	std::cout<<"Usage: ProxyServer <proxy port> <max thread>\n"; 
}

int main(int argc, char** argv)
{
	if(argc!=3) 
	{
		printusage();
		exit(-1);
	}
	int port;
	port = std::atoi(argv[1]);
	if(port<=0)
	{
		printf("Invaild port!\n");
		printusage();
		exit(-1);
	}

	int maxthread = std::atoi(argv[2]);
	if(maxthread<=0) 
	{
		printf("Invaild num thread!\n");
		printusage();
		exit(-1);
	}
	
	printf("[Server] Max thread: %d\n",maxthread);
	printf("[Server] Listen at port %d\n",port);
	
	http::InitSocket();
	ServicePool pool(maxthread);

	int yes = 1; 


	Socket server;
	if(!server.Listen(port))
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

