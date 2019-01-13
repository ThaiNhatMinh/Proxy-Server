#include "pch.h"
#include "util.h"
#include <sstream>
#include <string.h>
namespace http
{
	std::string BuildRequest(std::string host, std::string page)
	{

		std::string result;
		result += "GET ";
		//result += "/";
		result += page;
		result += " HTTP/1.1\r\n";
		result += "Host: ";
		result += host;
		result += "\r\n";
		result += "User-Agent: Mozilla/4.0\r\n\r\n";
		return result;
	}
	std::string ToString(response r)
	{
		std::stringstream ss;
		ss << r.statusCode;
		std::string t;
		t += r.protocolVer + " ";
		t += ss.str() + " ";
		t += r.statusText + " ";
		t += "\r\n";
		for (auto el : r.headerField)
		{
			t += el.first;
			t += ": ";
			t += el.second;
			t += "\r\n";
		}

		t += "\r\n\r\n";
		t += r.Data;

		return t;
	}
	std::string GetHostNameFromURL(const std::string URL)
	{
		std::string result = URL;
		std::size_t pos = result.find("http://");
		if (pos != std::string::npos)
		{
			result = result.erase(0, 7);
		}
		pos = result.find("https://");
		if (pos != std::string::npos)
		{
			result = result.erase(0, 8);
		}
		pos = result.find_first_of("/");
		result = result.substr(0, pos);
		pos = result.find("www.");
		if (pos != std::string::npos)
		{
			result = result.erase(0, 4);
		}
		return result;
	}

	std::string GetIPFromURL(const std::string URL)
	{
		std::string r;
		std::string hostname = GetHostNameFromURL(URL);
		struct hostent *hent;
		char ip[32];
		if ((hent = gethostbyname(hostname.c_str())) == NULL)
		{
			std::cerr << "Can't get ip" << std::endl;
#ifdef __linux__
			std::cout << strerror(errno) << std::endl;
#elif defined(WIN32)
			auto dwError = WSAGetLastError();
			if (dwError != 0)
			{
				if (dwError == WSAHOST_NOT_FOUND)printf("Host not found\n");
				else if (dwError == WSANO_DATA)	printf("No data record found\n");
				else printf("Function failed with error: %ld\n", dwError);
			}
#endif
			return r;
		}

		int i = 0;
		while (hent->h_addr_list[i] != 0) {
			if (inet_ntop(AF_INET, (void *)hent->h_addr_list[i], ip, sizeof(ip)) != NULL)
				//printf("\tIP Address #%d: %s\n", i, ip);
				r = ip;
			i++;
		}

		r = ip;
		return r;
	}

	std::string GetURIFromURL(const std::string URL)
	{
		std::string r = URL;

		std::string host = GetHostNameFromURL(URL);

		std::size_t pos = URL.find(host);
		r.erase(0, pos);
		r.erase(0, host.length());

		return r;
	}


	void InitSocket()
	{
#ifdef WIN32
		static WSADATA wsaData;
		int iResult;

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			std::cerr << "WSAStartup failed: " << iResult;
			return;
		}
#endif
	}

	void DestroySocket()
	{
#ifdef WIN32
		WSACleanup();
#endif
	}

	response ParseResponse(std::string data)
	{
		response r;
		if (data.substr(0, 6) == "<html>")
		{
			r.statusCode = 400;
			r.statusText = "Bad Request";
			return r;
		}

		std::size_t pos = data.find_first_of(' ');
		r.protocolVer = data.substr(0, pos);
		data.erase(0, pos + 1);

		pos = data.find_first_of(' ');
		r.statusCode = std::atoi(data.substr(0, pos).c_str());
		data.erase(0, pos + 1);

		pos = data.find_first_of("\r\n");
		r.statusText = data.substr(0, pos);
		data.erase(0, pos + 2);

		pos = data.find("\r\n\r\n");
		r.Data = data.substr(pos + 4);
		data.erase(pos, 0xffffffff);

		do
		{
			pos = data.find_first_of(':');
			if (pos == std::string::npos) break;
			std::string header = data.substr(0, pos);
			data.erase(0, pos + 2);

			pos = data.find_first_of('\n');
			std::string headerData = data.substr(0, pos - 1);
			data.erase(0, pos + 1);


			r.headerField.insert(std::pair< std::string, std::string>(header, headerData));


		} while (1);

		return r;
	}

	request ParseRequest(std::string data)
	{
		request r;

		std::size_t pos = data.find_first_of(' ');
		r.method = data.substr(0, pos);
		data.erase(0, pos + 1);

		pos = data.find_first_of(' ');

		r.URI = data.substr(0, pos);

		if (r.method == "CONNECT")
		{
			pos = r.URI.find_first_of(':');
			auto temp = r.URI.substr(pos + 1);
			r.port = std::atoi(temp.c_str());
			r.URI.erase(pos, 5);
			data.erase(0, pos +temp.length()+ 1);
		}
		else
		{
			data.erase(0, pos + 1);
			r.port = 80;
		}
		

		pos = data.find_first_of("\r\n");
		r.protocolVer = data.substr(0, pos);
		data.erase(0, pos + 2);

		do
		{
			pos = data.find_first_of(':');
			if (pos == std::string::npos) break;
			std::string header = data.substr(0, pos);
			data.erase(0, pos + 2);

			pos = data.find_first_of('\n');
			std::string headerData = data.substr(0, pos - 1);
			data.erase(0, pos + 1);


			r.headerField.insert(std::pair< std::string, std::string>(header, headerData));


		} while (1);

		return r;
	}

}
