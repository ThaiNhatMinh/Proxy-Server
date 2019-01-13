#pragma once
#include <string>
#include <vector>

namespace http
{
	struct response;
}
namespace helper
{

	struct HTMLFile
	{
		std::string name;
		std::string URL;
	};

	bool					CheckIfFolder(const std::string data);
	std::vector<HTMLFile>	GetAllFile(const std::string data);
	bool					SaveFileToDisk(const std::string folder, const std::string name, std::string data);
	std::string				GetFolderFromURL(std::string URL);
	http::response			DownLoadFile(const std::string& ip,std::size_t port, const std::string& query);
}