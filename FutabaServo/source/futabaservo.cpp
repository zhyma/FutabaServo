#include "..\include\futabaservo.h"

FutabaServo::FutabaServo()
{
	mPort = new SerialPort();
}

char FutabaServo::calcSum(unsigned char * data, int len)
{
	char sum = 0;
	for (int i = 0; i < len; ++i)
	{
		sum ^= data[i];
	}
	return sum;
}

void FutabaServo::shortPack(int type)
{
	//set target ID
	switch (type)
	{
	case INIT:
		ID = 0xff;
		break;
	default:
		ID = target_ID;
		break;
	}
	//content has been set already
	//calculate the sum
	CMD.data[CMD.len - 1] = calcSum(&ID, CMD.len - 5);
	if(RPU==true)
	{
		mPort->writePort(rs303[type].data, rs303[type].len);
	}
	else
	{
		mPort->writePort(rs303[type].data+2, rs303[type].len-2);
	}
	Sleep(100);
	return;
}

void FutabaServo::readPack(int type)
{
	//request all servo to reply with certain information
	switch (type)
	{
	case READID:
	case READINFO:
		ID = 0xff;
		break;
	default:
		ID = target_ID;
		break;
	}
	CMD.data[CMD.len-2] = calcSum(&ID, CMD.len-6);
	if(RPU==true)
	{
		mPort->writePort(CMD.data, CMD.len);
	}
	else
	{
		mPort->writePort(CMD.data+2, CMD.len-3);
	}
	return;
}

void FutabaServo::reset()
{
	shortPack(INIT);
	target_ID = 1;
	reverse = DEFAULT;
	shortPack(WRITE);

	return;
}

void FutabaServo::setID(int newID)
{
	//set new ID
	rs303[SETID].data[4] = target_ID;
	rs303[SETID].data[9] = newID;
	shortPack(SETID);

	target_ID = newID;
	//write to ROM (saved after reboot)
	shortPack(WRITE);

	return;
}

void FutabaServo::torqueOn()
{
	shortPack(TORQUEON);
	return;
}

int FutabaServo::readID()
{
	unsigned char readBuff[20] = { 0 };
	readPack(READID);
	mPort->readPort(readBuff, 20);
	for (int i = 0; i < 10; i++)
	{
		if (readBuff[i] == 0xfd && readBuff[i + 1] == 0xdf)
		{
			target_ID = readBuff[i+2];
			return target_ID;
		}
	}
	return READERROR;
}

//check the rotation direction is reversed or not
int FutabaServo::readRev()
{
	unsigned char readBuff[20] = { 0 };
	readPack(READREV);
	mPort->readPort(readBuff, 20);
	for (int i = 0; i < 10;i++)
	{ 
		if (readBuff[i] == 0xfd && readBuff[i + 1] == 0xdf)
		{
			if (readBuff[i+7] == 1)
			{
				return REVERSED;
			}
			else
			{
				return DEFAULT;
			}
		}
	}
	return READERROR;
}

//set the reverse of the servo
void FutabaServo::setRev(bool reverse)
{
	//1: reversed, 0: normal
	if (reverse == true)
	{
		rs303[SETREV].data[9] = 0x01;
	}
	else
	{
		rs303[SETREV].data[9] = 0x00;
	}
	shortPack(SETREV);
	Sleep(100);

	//write to ROM (saved after reboot)
	shortPack(WRITE);
	Sleep(100);

	return;
}

//pos: in degree, such as right angle==90, time: in ms
void FutabaServo::go2Pos(int pos, int time)
{
	//do position calculation
	rs303[SETPOS].data[9] = (pos * 10) & 0xff;//L
	rs303[SETPOS].data[10] = ((pos * 10) >> 8)  & 0xff;//H
	//do time calculation
	rs303[SETPOS].data[11] = (time / 10) & 0xff;//L
	rs303[SETPOS].data[12] = (time / 10 >> 8) & 0xff;//H

	//check sum
	shortPack(SETPOS);
	return;
}

void FutabaServo::testRun()
{
	//test run
	go2Pos(0, 100);
	Sleep(200);
	go2Pos(30, 100);
	Sleep(200);
	go2Pos(-30, 100);
	Sleep(200);
	go2Pos(0, 100);
}