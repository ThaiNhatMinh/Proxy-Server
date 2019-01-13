#pragma once
#include <string>
#include <map>


namespace http
{
	struct response
	{
		// Status line
		std::string protocolVer;
		int			statusCode;
		std::string statusText;

		// Header
		std::map<std::string, std::string> headerField;

		std::string Data;

	};

	struct request
	{
		bool isValid;
		std::string method;
		std::string protocolVer;
		std::string URI;
		int port;
		std::map<std::string, std::string> headerField;
	};
	std::string BuildRequest(std::string host, std::string page);
	std::string ToString(response r);
	std::string GetHostNameFromURL(const std::string URL);
	std::string GetIPFromURL(const std::string URL);
	std::string GetURIFromURL(const std::string URL);
	void		InitSocket();
	void		DestroySocket();

	response	ParseResponse(std::string data);
	request		ParseRequest(std::string data);
}
