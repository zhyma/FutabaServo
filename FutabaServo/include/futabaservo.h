#pragma once
#include <windows.h>

#include "..\include\serial.h"

#define READERROR	-1
//#define NORMAL		0
//for rotating direction
#define DEFAULT		0
#define REVERSED	1
#define UNKNWON		-1

#define CMD			rs303[type]//some commands of the servo
#define ID			CMD.data[4]//pointer to the ID of the command
//write command
#define INIT		0
#define WRITE		1
#define REBOOT		2
#define SETID		3
#define SETREV		4
#define SETPOS		5
#define TORQUEON	6
#define TORQUEOFF	7
//read command
#define READID		8	
#define READREV		9
#define READINFO	10


class FutabaServo {

public:
	SerialPort *mPort;

	int target_ID = 0xff;
	int reverse = UNKNWON;
	bool RPU = true;

	FutabaServo();

	void reset();
	void setID(int new_ID);
	void torqueOn();
	int  readID();
	int  readRev();
	void setRev(bool reverse);
	void go2Pos(int pos, int time);
	void testRun();

private:
	typedef struct {
		int type;
		int len;
		unsigned char data[20];
	} command;


	command rs303[11] = {
		////connect your Futaba servo with your USB-RS232 port through RPU module
		////Care check_sum and ID num
		////| RPU_HEADER | LENGTH | FA | AF | ID | Flag | Adr | Len| Cnt | CHECK_SUM |
		/*command command_init	=*/		{ INIT,	    10, { 0x53, 0x08, 0xFA, 0xAF, 0xFF, 0x10, 0xFF, 0xFF, 0x00, 0x00 } },//Initialize ROM, reset everything to default
		/*command command_write	=*/		{ WRITE,    10, { 0x53, 0x08, 0xFA, 0xAF, 0x00, 0x40, 0xFF, 0x00, 0x00, 0x00 } },//write ROM
		/*command command_reboot=*/		{ REBOOT,   10, { 0x53, 0x08, 0xFA, 0xAF, 0x00, 0x20, 0xFF, 0x00, 0x00, 0x00 } },//reboot servo
		/*command command_setID	=*/		{ SETID,    11, { 0x53, 0x09, 0xFA, 0xAF, 0x01, 0x00, 0x04, 0x01, 0x01, 0x00, 0x00 } },//change ID
		/*command command_setRev	=*/ { SETREV,   11, { 0x53, 0x09, 0xFA, 0xAF, 0x00, 0x00, 0x05, 0x01, 0x01, 0x00, 0x00 } },//set servo reverse
		/*command command_setPos	=*/ { SETPOS,   14, { 0x53, 0x0c, 0xFA, 0xAF, 0x00, 0x00, 0x1E, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 } }, //short package for Goal Position with Goal time
		//if not using RPU, need to send torque on for position-type command
		/*command command_torqueOn  =*/ {TORQUEON,  11, { 0x53, 0x09, 0xFA, 0xAF, 0xFF, 0x00, 0x24, 0x01, 0x01, 0x01, 0xDA } },
		//turn the torque on. This command will be send to 0xff (all) by default when applying a RPU
		//use this command first is you are connecting to servos directly
		/*command command_torqueOff =*/ {TORQUEOFF, 11, { 0x53, 0x09, 0xFA, 0xAF, 0xFF, 0x00, 0x24, 0x01, 0x01, 0x00, 0xDB } },

		//for those commands that will request a reply package, an extra footer will be needed (8+Len)
		////| RPU_HEADER | LENGTH | FA | AF | ID | | Flag | Adr | Len| Cnt | CHECK_SUM | RPU_FOOTER
		//command that will receive response from servo, with 2 bytes header and 1 byte footer for RPU
		/*command command_readID   =*/ { READID, 11, { 0x54, 0x09, 0xFA, 0xAF, 0xFF, 0x0F, 0x04, 0x01, 0x00, 0x00, 0x09 } },//read servo ID
		/*command command_readRev  =*/ { READREV, 11, { 0x54, 0x09, 0xFA, 0xAF, 0x00, 0x0F, 0x05, 0x01, 0x00, 0x00, 0x09 } },//read servo reverse
		/*command command_readInfo =*/ { READINFO, 11, { 0x54, 0x09, 0xFA, 0xAF, 0xFF, 0x0F, 0x04, 0x02, 0x00, 0x00, 0x0A } },//read servo ID and reverse
	};

	char calcSum(unsigned char* data, int len);//check_sum
	void shortPack(int type);
	void readPack(int type);
};