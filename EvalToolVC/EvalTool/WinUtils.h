#pragma once
#include <vector>

namespace WinUtil
{
	std::vector<std::string> GetDirectries(const std::string& dir_path, const std::string& filter = "");
	std::vector<std::string> GetFiles(const std::string& dir_path, const std::string& filter = "");

	void MySleep(unsigned long int msec);
}