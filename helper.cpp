#include "pch.h"
#include "helper.h"
#include "Socket.h"
#include "util.h"
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <string.h>

namespace fs = std::experimental::filesystem;

namespace helper
{

	bool CheckIfFolder(const std::string data)
	{
		std::size_t beginTag, endTag;
		std::string content;
		beginTag = data.find("<title>");
		endTag = data.find("</title>");
		content = data.substr(beginTag + 7, endTag - beginTag);
		if (content.find("Index of /") == std::string::npos) return false;

		return true;
	}

	std::vector<HTMLFile> GetAllFile(const std::string data)
	{
		std::vector<HTMLFile> r;
		std::size_t beginTag, endTag;
		std::string content;

		// nginx server
		if (data.find("Apache Server at") == std::string::npos) 
		{
			beginTag = data.find("<table>");
			endTag = data.find("</table>");
			std::string table = data.substr(beginTag + 7, endTag - beginTag);


			int i = 0;
			while (1)
			{
				beginTag = table.find("<tr>");
				endTag = table.find("</tr>");
				if (beginTag == std::string::npos || endTag == std::string::npos) break;
				std::string row = table.substr(beginTag + 4, endTag - beginTag);


				table.erase(beginTag, endTag - beginTag + 5);

				i++;
				if (i < 4) continue; // ignore table header, break line and back link

				HTMLFile file;

				beginTag = row.find("<td><a href=\"");
				endTag = row.find("</a></td>");
				row = row.substr(beginTag + 13, endTag - beginTag - 13);


				file.URL = row.substr(0, row.find("\">"));

				file.name = row.substr(row.find("\">") + 2);

				r.push_back(file);


			}
			r.pop_back();
		}
		else //Apache Server
		{
			beginTag = data.find("<pre>");
			endTag = data.find("</pre>");
			std::string table = data.substr(beginTag + 5, endTag - beginTag);
			int i = 0;
			while (1)
			{
				beginTag = table.find("<a href=\"");
				endTag = table.find("</a>");
				if (beginTag == std::string::npos || endTag == std::string::npos) break;
				std::string row = table.substr(beginTag + 9, endTag - beginTag-9);
				table.erase(beginTag, endTag - beginTag + 4);
				i++;
				if (i < 6) continue;

				HTMLFile file;

				file.URL = row.substr(0, row.find("\">"));

				file.name = row.substr(row.find("\">") + 2);

				if (file.name.find("..&gt;") != std::string::npos) // short name
				{
					file.name = file.URL;
					std::size_t pos;
					while ((pos = file.name.find("%20"))!=std::string::npos)
					{
						file.name.erase(pos, 3);
						file.name.insert(pos, 1, ' ');
					}
				}

				r.push_back(file);
			}
		}

		return r;
	}

	bool SaveFileToDisk(const std::string folder, const std::string name, std::string data)
	{
		if (folder.length()>0 && !fs::exists(folder))
		{
			fs::create_directories(folder);
		}

		std::string fullFile;
		if(folder.length()>0) fullFile = folder + "/" + name;
		else fullFile = name;

		std::ofstream of(fullFile, std::ios::binary);
		if (!of.is_open())
		{
			std::cout << "Can't create file: " << name << std::endl;
			return 0;
		}

		of.write(data.c_str(), data.length());

		of.close();

		return true;
	}

	std::string GetFolderFromURL(std::string URL)
	{
		std::string r;

		if (URL[URL.length() - 1] == '/')
		{
			URL.erase(URL.length() - 1, 0xffffff);
		}

		r = URL.substr(URL.find_last_of('/') + 1);


		return r;
	}

	http::response DownLoadFile(const std::string & ip, std::size_t port, const std::string & query)
	{
		http::response r;
		Socket socketFile;
		if (!socketFile.Connect(ip, port))
			return r;

		socketFile.Send(query);

		std::string dataFile;

		long total_recv = 0;
		long total_length = 0;
		long file_length = 0;

		//download header
		while (1)
		{
			std::string data;
			long length = socketFile.Recv(data);
			if (length > 0)
			{
				dataFile += data;
				total_recv += length;
			}

			std::size_t pos = dataFile.find("\r\n\r\n");
			if (pos != std::string::npos)
			{
				r = http::ParseResponse(data);
				file_length = std::atol(r.headerField["Content-Length"].c_str());
				r.Data.resize(file_length, 0);
				total_length = pos + file_length + 4;

				dataFile.erase(0, pos + 4);
				break;
			}
			if (total_recv >= total_length) break;
		}

		if (r.statusCode != 200) return r;


		// download data
		while (1)
		{
			std::string data;
			long length = socketFile.Recv(data);
			if (length > 0)
			{
				dataFile += data;
				total_recv += length;
				
			}
			if (total_recv >= total_length) break;
		}
		memcpy(&r.Data[0], &dataFile[0], dataFile.length());
		return r;
	}

}