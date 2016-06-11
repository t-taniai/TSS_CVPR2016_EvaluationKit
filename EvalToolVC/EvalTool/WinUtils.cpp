#include "WinUtils.h"

#include <vector>
#include <algorithm>
#include <windows.h>
#include <time.h>

namespace WinUtil
{
	std::vector<std::string> GetFiles(const std::string& dir_path, const std::string& filter)
	{
		WIN32_FIND_DATAA fd;

		std::string ss = dir_path + filter;
		HANDLE hFind = FindFirstFileA(ss.c_str(), &fd);

		std::vector<std::string> fileList;

		if (hFind == INVALID_HANDLE_VALUE)
			return fileList;

		do
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				fileList.push_back(fd.cFileName);
		} while (FindNextFileA(hFind, &fd));

		FindClose(hFind);

		auto newend = std::remove_if(fileList.begin(), fileList.end(), [](const std::string& str){ return str == "." || str == ".."; });
		fileList.erase(newend, fileList.end());

		return fileList;
	}
	std::vector<std::string> GetDirectries(const std::string& dir_path, const std::string& filter)
	{
		WIN32_FIND_DATAA fd;

		std::string ss = dir_path + filter;
		HANDLE hFind = FindFirstFileA(ss.c_str(), &fd);

		std::vector<std::string> fileList;

		if (hFind == INVALID_HANDLE_VALUE)
			return fileList;
		do
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				fileList.push_back(fd.cFileName);
		} while (FindNextFileA(hFind, &fd));

		FindClose(hFind);

		auto newend = std::remove_if(fileList.begin(), fileList.end(), [](const std::string& str){ return str == "." || str == ".."; });
		fileList.erase(newend, fileList.end());

		return fileList;
	}

	void MySleep(unsigned long int msec)
	{
		//Sleep(msec);
		clock_t st = clock();
		while (1000 * (clock() - st) / CLOCKS_PER_SEC < msec)
			;
	}
}