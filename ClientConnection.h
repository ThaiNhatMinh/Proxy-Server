#pragma once
#include <thread>
#include <vector>
#include <memory>
#include "Socket.h"
#ifdef __linux__
#include <poll.h>
#endif
#include "EventManager.h"

#include <map>
class ClientConnection
{
public:
	typedef std::unique_ptr<ClientConnection> Ptr;
	enum State
	{
		STOP,
		RUNNING
	};
private:
	struct InternalData
	{
		Socket sk;
		std::string buffer_write; // buffer will write to socket
		std::string buffer_read; // buffer will read from to socket
		InternalData()=default;
		InternalData(Socket&& sk)
		{
			this->sk = std::move(sk);
			//std::cout<<"sk " << this->sk.GetHandle()  << " other " << sk.GetHandle() << std::endl;
		}

		InternalData(InternalData&& other)
		{
			sk = std::move(other.sk);
			buffer_write = std::move(other.buffer_write);
			buffer_read = std::move(other.buffer_read);
		}
		InternalData& operator=(InternalData&& other)
		{
			//std::cout<<"InternalData& operator=(InternalData&& other)\n";
			if(&other==this) return *this;
			sk = std::move(other.sk);
			//std::cout<<"sk " << sk.GetHandle()  << " other " << other.sk.GetHandle() << std::endl;
			buffer_write = std::move(other.buffer_write);
			buffer_read = std::move(other.buffer_read);
			return *this;
		}

	};
private:
	InternalData m_cl;
	std::map<std::string,InternalData> m_remotes; // map between host and socket
	std::vector<pollfd> m_polls;
	std::vector<std::string> m_ToRemote;
	std::thread m_Thread;
	State m_state;
	int id;
public:
	ClientConnection(int _id);

	void Active(Socket&& cl);
	void DeActive();
	State GetState();
private:
	void Process();

	bool CheckRequest(std::string method);
};