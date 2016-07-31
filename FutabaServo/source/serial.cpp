#include "..\include\serial.h"


int SerialPort::look4Port(std::string *comList)
{
	int i = 0;
	char name[100];
	unsigned char szPortName[50];
	long status;
	DWORD dwIndex = 0;
	DWORD dwName;
	DWORD dwSizeofPortName;
	DWORD type;
	HKEY hKey;
	char dataSet[]  = "HARDWARE\\DEVICEMAP\\SERIALCOMM\\";
	dwName = sizeof(name);
	dwSizeofPortName = sizeof(szPortName);
	long ret0 = RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCSTR)dataSet, 0, KEY_READ, &hKey); 
	if (ret0 == ERROR_SUCCESS)
	{
		do
		{
			status = RegEnumValue(hKey, dwIndex++, name, &dwName, NULL, &type, szPortName, &dwSizeofPortName);
			if ((status == ERROR_SUCCESS) || (status == ERROR_MORE_DATA))
			{
				comList[i] = reinterpret_cast<char*>(szPortName);
				i++;
			}
			dwName = sizeof(name);
			dwSizeofPortName = sizeof(szPortName);
		} while ((status == ERROR_SUCCESS) || (status == ERROR_MORE_DATA));
		RegCloseKey(hKey);
	}
	return i;
}

bool SerialPort::openPort()
{
	hCom = CreateFile(activePort,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (hCom == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, _T("Port open failed"), _T("Error"), MB_OK);
		return false;
	}

	SetupComm(hCom, 1024, 1024);
	COMMTIMEOUTS TimeOuts;
	//read
	TimeOuts.ReadIntervalTimeout = 1000;
	TimeOuts.ReadTotalTimeoutMultiplier = 500;
	TimeOuts.ReadTotalTimeoutConstant = 5000;
	//write
	TimeOuts.WriteTotalTimeoutMultiplier = 500;
	TimeOuts.WriteTotalTimeoutConstant = 2000;
	SetCommTimeouts(hCom, &TimeOuts);

	DCB dcb;
	GetCommState(hCom, &dcb);
	dcb.BaudRate = CBR_115200;
	dcb.ByteSize = 8;
	//flow control : none
	dcb.fParity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	SetCommState(hCom, &dcb);

	PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR);
	return true;
}

bool SerialPort::readPort(unsigned char* readBuff, int len)
{
	DWORD dwBytesRead = len;
	BOOL bReadStat;
	bReadStat = ReadFile(hCom, readBuff, len, &dwBytesRead, NULL);
	PurgeComm(hCom, PURGE_RXCLEAR);
	//hCom
	return true;
}

bool SerialPort::writePort(unsigned char* command, int len)
{
	DWORD dwBytesWrite = len;
	BOOL bWriteStat;
	PurgeComm(hCom, PURGE_TXCLEAR);
	bWriteStat = WriteFile(hCom, command, len, &dwBytesWrite, NULL);
	if (!bWriteStat)
	{
		MessageBox(NULL, _T("Send failed"), _T("Error"), MB_OK);
		return false;
	}
	return true;
}