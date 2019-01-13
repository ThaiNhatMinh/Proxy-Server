#pragma once

#include <string>

class Socket
{
public:
	

public:
	Socket();
	~Socket();
	Socket(Socket&&);
	Socket& operator=(Socket&&);

	bool	Connect(std::string ip,std::size_t port);
	bool	ConnectToHost(std::string host, std::size_t port);
	bool	Listen(std::size_t port);
	
	int		Send(const std::string& buffer);
	int		RecvAll(std::string& data, int timeout);
	int		Recv(std::string& data);
	Socket	Accept(sockaddr_in*);
	bool	IsValid();
	int		SetOpt(int level, int optname, const char* optval, int optlen);
	void	EnableNonblock();
	Socket(int handle);

	int		GetHandle();
private:

	int m_Handle;
};