#include "pch.h"
#include "ClientConnection.h"
#include "helper.h"
#include "util.h"
#include <chrono>
#if __linux__
#include <errno.h>
#include <string.h>
#endif

ClientConnection::ClientConnection(int _id)
{
	m_state = STOP;
	id = _id;
}

void ClientConnection::Active(Socket &&cl)
{
	m_cl = Socket();
	m_polls.clear();
	m_remotes.clear();
	std::cout<< "[ClientConnection][" <<id << "]" <<"[Active] Client socket id " << cl.GetHandle() << std::endl;
	m_cl.sk = std::move(cl);
	m_cl.sk.EnableNonblock();
	m_Thread = std::thread(&ClientConnection::Process, this);
	m_state = RUNNING;
	pollfd fd;
	fd.fd = m_cl.sk.GetHandle();
	fd.events = POLLOUT | POLLWRBAND | POLLIN;

	m_polls.push_back(fd);
	m_ToRemote.push_back("client");
}

void ClientConnection::DeActive()
{
	m_cl = Socket();
	m_polls.clear();
	m_remotes.clear();
	m_state = STOP;
	m_Thread.detach();
}
using namespace std::chrono_literals;

void ClientConnection::Process()
{
	// handle data from a client
	int nbytes;
	while (1)
	{
		int ret = poll(m_polls.data(), m_polls.size(), -1);

		if (ret <= 0)
		{
			std::this_thread::sleep_for(1s);
			
			continue;
		}

		//receive data from client
		if (m_polls[0].revents & POLLIN)
		{

			m_cl.buffer_read.clear();
			nbytes = m_cl.sk.Recv(m_cl.buffer_read);
			if (nbytes < 0)
			{
				printf("Recv Error: %s\n", strerror(errno));
			}
			else if (nbytes == 0)
			{
				// close
				std::cout << "[ClientConnection][" <<id << "]" << "Thread close...\n";
				break;
			}
			else
			{
				http::request request = http::ParseRequest(m_cl.buffer_read);
				std::cout <<"[ClientConnection][" <<id << "][Client]"<< "Data comming: " << m_cl.sk.GetHandle() << " size " <<  m_cl.buffer_read.size() << std::endl;
				//std::cout << "-------------------------------------------\n";
				//std::cout << m_cl.buffer_read << std::endl;
				std::string host;

				if (request.method == "CONNECT")
				{
					host = request.URI;
					//std::cout<<"Using URI: " << host << std::endl;
				}
				else
				{
					host = request.headerField["Host"];
					std::size_t pos = host.find(':');
					if(pos!=std::string::npos) host = host.substr(0,pos);
					
					//std::cout << "Parse Host: " << host << std::endl;
				}
				auto iterator = m_remotes.find(host);
				if (iterator != m_remotes.end())
				{
					std::cout<<"[ClientConnection][" <<id << "]" <<"Already connect to: " << host << std::endl;
					iterator->second.buffer_write = m_cl.buffer_read;
				}
				else
				{
					Socket remote;
					if (!remote.ConnectToHost(http::GetHostNameFromURL(host), request.port))
					{
						std::cout <<"[ClientConnection][" <<id << "]" << "Can't connect to: " << host << std::endl;
						http::response r;
						r.statusCode = 403;
						r.statusText = "Forbidden";
						r.protocolVer = "HTTP/1.1";
						std::string data = http::ToString(r);
						
					}
					else
					{
						std::cout<<"[ClientConnection][" <<id << "]" <<"Success connect to " << host << std::endl;
						remote.EnableNonblock();
						m_remotes.insert(std::pair<std::string,InternalData>(host,InternalData(std::move(remote))));
						m_remotes[host].buffer_write = m_cl.buffer_read;
						pollfd fd;
						fd.fd = m_remotes[host].sk.GetHandle();
						fd.events = POLLIN|POLLOUT;
						m_polls.push_back(fd);
						m_ToRemote.push_back(host);
						std::cout<<"[ClientConnection][" <<id << "]" <<"Host " << host<< " fd " << m_polls.back().fd << " remote " << m_remotes[host].sk.GetHandle() << std::endl;

					}
				}
			}
		}
		
		// write data to client
		if(m_polls[0].revents & POLLOUT) 
		{
			if(m_cl.buffer_write.size())
			{
				nbytes = m_cl.sk.Send(m_cl.buffer_write);
				if(nbytes<m_cl.buffer_write.length())
					m_cl.buffer_write = m_cl.buffer_write.substr(nbytes);
				else m_cl.buffer_write.clear();

				std::cout<<"[ClientConnection][" <<id << "]" <<"Send " << nbytes << "/" << m_cl.buffer_write.length() << " to client.\n";

			}
		}

		std::vector<std::vector<pollfd>::iterator> removePollfd;
		std::vector<std::vector<std::string>::iterator> removeMap;
		std::vector<pollfd>::iterator PollFDIterator = m_polls.begin();
		std::vector<std::string>::iterator MapIterator = m_ToRemote.begin();
		if(m_polls.size()==1) continue;

		for (int i = 1; i < m_polls.size(); i++,PollFDIterator++,MapIterator++)
		{
			auto r = m_remotes.find(m_ToRemote[i]);
			if (r == m_remotes.end())
			{
				std::cout << "[ClientConnection][" <<id << "]" <<"No key: " << m_ToRemote[i] << " found\n";
				continue;
			}
			auto remote = &r->second;
			if(m_polls[i].revents & POLLIN)
			{
				nbytes = remote->sk.Recv(remote->buffer_read);
				if(nbytes==0)
				{
					removePollfd.push_back(PollFDIterator);
					m_remotes.erase(m_ToRemote[i]);
					std::cout<<"[ClientConnection][" <<id << "]" <<" Remote close " << PollFDIterator->fd <<" " << remote->sk.GetHandle() << std::endl;
					continue;
				}
				else if(nbytes<0)
				{
					printf("Recv Error:%d %s\n", remote->sk.GetHandle(),strerror(errno));
					std::cout<< m_ToRemote[i] << std::endl;
					auto r = m_remotes.find(m_ToRemote[i]);
					if(r==m_remotes.end())
					{
						std::cout<<"No key: " << m_ToRemote[i] << "found\n";
					}else  std::cout<<"FD " << r->second.sk.GetHandle() << std::endl;

				}
				else
				{
					std::cout << "[ClientConnection][" <<id << "]" <<" Data comming from remote: " << remote->sk.GetHandle() << " size " <<remote->buffer_read.size()  << std::endl;
					std::cout << "[ClientConnection][" <<id << "]" <<" Data remain in client buffer " << m_cl.buffer_write.size() << std::endl;
					m_cl.buffer_write += remote->buffer_read;
				}
			}

			if(m_polls[i].revents & POLLOUT)
			{
				if(remote->buffer_write.size())
				{
					nbytes = remote->sk.Send(remote->buffer_write);
					if (nbytes < remote->buffer_write.length())
						remote->buffer_write = remote->buffer_write.substr(nbytes);
					else
						remote->buffer_write.clear();

					std::cout << "[ClientConnection][" <<id << "]" <<" Send " << nbytes << "/" << m_cl.buffer_write.length() << " to remote.\n";
				}
			}
		}
			

		
		for (auto &el : removePollfd)
		{
			std::cout<<"Removing fd " << (*el).fd << std::endl;
			m_polls.erase(el);
		}
		for (auto &el : removeMap)
		{
			std::cout<<"Removing map " << (*el) << std::endl;
			m_ToRemote.erase(el);
		}
	}

	DeActive();
}

ClientConnection::State ClientConnection::GetState() { return m_state; }