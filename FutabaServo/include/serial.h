#pragma once
#include <windows.h>
#include <tchar.h>
#include <string>

class SerialPort {
	
public:
	LPCTSTR activePort;
	HANDLE hCom;

	int  look4Port(std::string *comList);
	bool openPort();
	bool readPort(unsigned char* readBuff, int len);
	bool writePort(unsigned char* command, int len);
};