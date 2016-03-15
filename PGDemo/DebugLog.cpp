#include "stdafx.h"
#include "DebugLog.h"
#include <fstream>
#include <ctime>

std::string file_name = "";

void _CreateLog(std::string path)
{
	file_name = path+"\\pgdemo.log";
	std::ofstream log_file(file_name, std::ios::out);
	std::time_t t;
	std::time(&t);
	std::tm* data;
	data = std::localtime(&t);
	log_file << data->tm_mon << "/" << data->tm_mday << " " << data->tm_hour << ":" << data->tm_min << ":" << data->tm_sec << " log file created" << std::endl;
	log_file.close();
}

void _Write2LogS(std::string str)
{
	std::ofstream log_file(file_name, std::ios::app);
	std::time_t t;
	std::time(&t);
	std::tm* data;
	data = std::localtime(&t);
	log_file << data->tm_mon << "/" << data->tm_mday << " " << data->tm_hour << ":" << data->tm_min << ":" << data->tm_sec << " " << str.c_str()<<std::endl;
	log_file.close();
}

void _Write2LogS(std::wstring str)
{
	std::ofstream log_file(file_name, std::ios::app);
	std::time_t t;
	std::time(&t);
	std::tm* data;
	data = std::localtime(&t);
	log_file << data->tm_mon << "/" << data->tm_mday << " " << data->tm_hour << ":" << data->tm_min << ":" << data->tm_sec << " " << str.c_str() << std::endl;
	log_file.close();
}

void _Write2LogSI(std::string str, int i)
{
	std::ofstream log_file(file_name, std::ios::app);
	std::time_t t;
	std::time(&t);
	std::tm* data;
	data = std::localtime(&t);
	log_file << data->tm_mon << "/" << data->tm_mday << " " << data->tm_hour << ":" << data->tm_min << ":" << data->tm_sec << " " << str.c_str() << " "<<i<<std::endl;
	log_file.close();
}

void _Write2LogSF(std::string str, float f)
{
	std::ofstream log_file(file_name, std::ios::app);
	std::time_t t;
	std::time(&t);
	std::tm* data;
	data = std::localtime(&t);
	log_file << data->tm_mon << "/" << data->tm_mday << " " << data->tm_hour << ":" << data->tm_min << ":" << data->tm_sec << " " << str.c_str() << " " << f << std::endl;
	log_file.close();
}

void _Write2LogSF(std::string str, double d)
{
	std::ofstream log_file(file_name, std::ios::app);
	std::time_t t;
	std::time(&t);
	std::tm* data;
	data = std::localtime(&t);
	log_file << data->tm_mon << "/" << data->tm_mday << " " << data->tm_hour << ":" << data->tm_min << ":" << data->tm_sec << " " << str.c_str() << " " << d << std::endl;
	log_file.close();
}