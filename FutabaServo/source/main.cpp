#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include "..\resource\resource.h"

#include <string>

#include "..\include\serial.h"
#include "..\include\futabaservo.h"

//Project->Properties->Configuration Properties->General->Character Set->Not Set

#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")

#pragma comment(lib, "ComCtl32.lib")


HINSTANCE hInst;//current instance
FutabaServo *mServo;

INT_PTR CALLBACK ChangeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ReverseProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void btnConnect(HWND hDlg)
{
	char comPort[5] = {};
	GetDlgItemText(hDlg, IDC_COMBOCOM, comPort,5);

	mServo->mPort->activePort = comPort;
	mServo->mPort->openPort();

	if (mServo->mPort->hCom != NULL)
	{
		std::string str(comPort);
		str = str + " opened";
		SendMessage(GetDlgItem(hDlg, IDC_STATUS), SB_SETTEXT, 0, (LPARAM)str.c_str());
		EnableWindow(GetDlgItem(hDlg, IDC_CONNECT), false);
		EnableWindow(GetDlgItem(hDlg, IDC_READ), true);
		EnableWindow(GetDlgItem(hDlg, IDC_RESET), true);
	}
	return;
}

void btnRead(HWND hDlg)
{
	//read the ID and reverse status of the servo
	int id = mServo->readID();
	if (mServo->RPU != true)
	{
		mServo->torqueOn();
	}
	std::string str;
	if (id != READERROR)
	{
		//get ID from servo
		str = std::to_string(mServo->target_ID);
	}
	else
	{
		//failed
		str = "?";
	}
	str = "Servo ID: " + str;
	SendMessage(GetDlgItem(hDlg, IDC_STATUS), SB_SETTEXT, 1, (LPARAM)str.c_str());
	EnableWindow(GetDlgItem(hDlg, IDC_CHANGE), true);
	EnableWindow(GetDlgItem(hDlg, IDC_REVERSE), true);
	EnableWindow(GetDlgItem(hDlg, IDC_ZERO), true);
	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER1), true);

	int reversed = mServo->readRev();
	if (reversed==REVERSED)
	{
		SendMessage(GetDlgItem(hDlg, IDC_STATUS), SB_SETTEXT, 3, (LPARAM)"Reversed: Yes");
		mServo->reverse = REVERSED;
	}
	else if(reversed==DEFAULT)
	{
		SendMessage(GetDlgItem(hDlg, IDC_STATUS), SB_SETTEXT, 3, (LPARAM)"Reversed: No");
		mServo->reverse = DEFAULT;
	}
	else
	{
		SendMessage(GetDlgItem(hDlg, IDC_STATUS), SB_SETTEXT, 3, (LPARAM)"Reversed: ?");
	}
	return;
}

void btnReset(HWND hDlg)
{
	//reset all servos to default setting
	mServo->reset();
	if (mServo->RPU != true)
	{
		mServo->torqueOn();
	}
	std::string str = "Servo ID: 1";
	SendMessage(GetDlgItem(hDlg, IDC_STATUS), SB_SETTEXT, 1, (LPARAM)str.c_str());
	SendMessage(GetDlgItem(hDlg, IDC_STATUS), SB_SETTEXT, 3, (LPARAM)"Reversed: No");
	mServo->testRun();
	EnableWindow(GetDlgItem(hDlg, IDC_CHANGE), true);
	EnableWindow(GetDlgItem(hDlg, IDC_REVERSE), true);
	EnableWindow(GetDlgItem(hDlg, IDC_ZERO), true);
	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER1), true);
	return;
}

void btnRev(HWND hDlg)
{
	//open a new dialog for reverse
	HWND newhDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_REVERSE), hDlg, ReverseProc);
	ShowWindow(newhDlg, SW_SHOW);
	return;
}

void btnChange(HWND hDlg)
{
	//reverse sign will be cleaned as well
	std::string str = std::to_string(mServo->target_ID);
	//open a new dialog to change the ID
	HWND newhDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_CHANGEID), hDlg, ChangeProc);
	ShowWindow(newhDlg, SW_SHOW);
	//set current ID
	SetWindowText(GetDlgItem(newhDlg, IDC_CURRENTID), str.c_str());
	return;
}

void scrollChange(HWND hDlg)
{
	LRESULT pos = SendMessage(GetDlgItem(hDlg, IDC_SLIDER1), TBM_GETPOS, 0, 0);

	std::string str = std::to_string((int)pos);
	str = "Horn Pos: " + str;
	SendMessage(GetDlgItem(hDlg, IDC_STATUS), SB_SETTEXT, 2, (LPARAM)str.c_str());
	//send command to servo
	mServo->go2Pos(pos, 100);
}

void btnZero(HWND hDlg)
{
	SendMessage(GetDlgItem(hDlg, IDC_STATUS), SB_SETTEXT, 2, (LPARAM)"Horn Pos: 0");
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER1), TBM_SETPOS, TRUE, 0);
	mServo->go2Pos(0, 100);
}

void sliderInit(HWND hDlg)
{
	HWND hwndTrackbar = GetDlgItem(hDlg, IDC_SLIDER1);
	SendMessage(hwndTrackbar, TBM_SETRANGE, TRUE, MAKELONG(-150, 150));
	SendMessage(hwndTrackbar, TBM_SETPOS, TRUE, LPARAM(0));
	SendMessage(hwndTrackbar, TBM_SETTICFREQ, 45, 0);
	SendMessage(hwndTrackbar, TBM_SETPAGESIZE, 0, LPARAM(30));
	HWND hwndBuddy = CreateWindowEx(0, "STATIC", "-150", SS_RIGHT | WS_CHILD | WS_VISIBLE, 0, 0, 35, 20, hDlg, NULL, NULL, NULL);
	SendMessage(hwndTrackbar, TBM_SETBUDDY, (WPARAM)TRUE, (LPARAM)hwndBuddy);
	hwndBuddy = CreateWindowEx(0, "STATIC", "+150", SS_LEFT | WS_CHILD | WS_VISIBLE, 0, 0, 35, 20, hDlg, NULL, NULL, NULL);
	SendMessage(hwndTrackbar, TBM_SETBUDDY, (WPARAM)FALSE, (LPARAM)hwndBuddy);
}

void createStatusBar(HWND hDlg)
{
	HWND hwndStatus = CreateWindowEx(
			0,                       // no extended styles
			STATUSCLASSNAME,         // name of status bar class
			(PCTSTR)NULL,           // no text when first created
			SBARS_SIZEGRIP |         // includes a sizing grip
			WS_CHILD | WS_VISIBLE,   // creates a visible child window
			0, 0, 0, 0,              // ignores size and position
			hDlg,					// handle to parent window
			(HMENU)IDC_STATUS,       // child window identifier
			NULL,                   // handle to application instance
			NULL);                   // no window creation data

	// Get the coordinates of the parent window's client area.
	RECT rcClient;
	GetClientRect(hDlg, &rcClient);

	// Allocate an array for holding the right edge coordinates.
	//4: COM status, servoID, horn position, reversed?
	int cParts = 4;
	HLOCAL hloc = LocalAlloc(LHND, sizeof(int) * cParts);
	PINT paParts = (PINT)LocalLock(hloc);

	// Calculate the right edge coordinate for each part, and
	// copy the coordinates to the array.
	int nWidth = rcClient.right / cParts;
	int rightEdge = nWidth;
	for (int i = 0; i < cParts; i++)
	{
		paParts[i] = rightEdge;
		rightEdge += nWidth;
	}

	// Tell the status bar to create the window parts.
	SendMessage(hwndStatus, SB_SETPARTS, (WPARAM)cParts, (LPARAM)paParts);
	SendMessage(hwndStatus, SB_SETTEXT, 0, (LPARAM)"COM closed");
	SendMessage(hwndStatus, SB_SETTEXT, 1, (LPARAM)"Servo ID: ?");
	SendMessage(hwndStatus, SB_SETTEXT, 2, (LPARAM)"Horn Pos: 0");
	SendMessage(hwndStatus, SB_SETTEXT, 3, (LPARAM)"Reversed: ?");

	// Free the array, and return.
	LocalUnlock(hloc);
	LocalFree(hloc);
	return;
}

INT_PTR CALLBACK MainProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int rpu;
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CONNECT:
			//connect to COM port
			rpu = MessageBox(hDlg, TEXT("Connected through RPU?"), TEXT("RPU"), MB_ICONQUESTION | MB_YESNOCANCEL);
			if (rpu == IDYES)
			{
				btnConnect(hDlg);
				mServo->RPU = true;
			}
			else if (rpu == IDNO)
			{
				btnConnect(hDlg);
				mServo->RPU = false;
			}
			return TRUE;
		case IDC_CHANGE:
			//write new ID to this servo
			btnChange(hDlg);
			return TRUE;
		case IDC_ZERO:
			//reset the position of servo
			btnZero(hDlg);
			return TRUE;
		case IDC_READ:
			btnRead(hDlg);
			return TRUE;
		case IDC_REVERSE:
			if (mServo->reverse != UNKNWON)
			{
				btnRev(hDlg);
			}
			return TRUE;
		case IDC_RESET:
			btnReset(hDlg);
			return TRUE;
		}
		break;

	case WM_HSCROLL:
		//turning the horn
		switch (wParam)
		{
		//only send command after left button released
		case TB_ENDTRACK:
			scrollChange(hDlg);
			break;
		}
		break;

	case WM_CLOSE:
		if (mServo->mPort->hCom != NULL)
		{
			CloseHandle(mServo->mPort->hCom);
		}
		DestroyWindow(hDlg);
		return TRUE;

	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	}

	return FALSE;
}

INT_PTR CALLBACK ChangeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			BOOL translated;
			int newID = GetDlgItemInt(hDlg, IDC_EDITID, &translated, FALSE);
			if (newID==0)
			{
				MessageBox(NULL, _T("Please input an ID"), _T("Error"), MB_OK);
				return TRUE;
			}
			//change ID
			mServo->setID(newID);
			HWND parentDlg = GetParent(hDlg);
			std::string str = std::to_string(mServo->target_ID);
			str = "ServoID: " + str;
			SendMessage(GetDlgItem(parentDlg, IDC_STATUS), SB_SETTEXT, 1, (LPARAM)str.c_str());
			mServo->testRun();
			EndDialog(hDlg, IDOK);
			return TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			//cancle
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		break;

	case WM_CLOSE:
			DestroyWindow(hDlg);
		return TRUE;

	case WM_DESTROY:
		EndDialog(hDlg, IDCANCEL);
		return TRUE;
	}

	return FALSE;
}

INT_PTR CALLBACK ReverseProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPDRAWITEMSTRUCT pDIS = (LPDRAWITEMSTRUCT)lParam;
	switch (uMsg)
	{
	case WM_DRAWITEM:
		if (mServo->reverse != UNKNWON)
		{
			HICON hIcon;
			if (pDIS->CtlID == IDC_STATUS1)//the image on the left hand side (current status)
			{
				if(mServo->reverse == DEFAULT)
					hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DEFAULT), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
				else
					hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_REVERSED), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
			}
			if (pDIS->CtlID == IDC_STATUS2)//the imamge on the right hand side (status after reversing)
			{
				if(mServo->reverse == DEFAULT)
					hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_REVERSED), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
				else
					hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DEFAULT), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
			}
			if (pDIS->CtlID == IDC_ARROWMARK)
			{
				hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ARROW), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
			}
			DrawIconEx(pDIS->hDC, 0, 0, hIcon, 0, 0, 0, NULL, DI_NORMAL);
			DestroyIcon(hIcon);
			return TRUE;
		}
		else//current status unknown
		{
			//send command to servo to get current status
			int status = mServo->readRev();
			if (status != READERROR)
				mServo->reverse = status;
			return TRUE;
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			HWND parentDlg = GetParent(hDlg);

			if (mServo->reverse == DEFAULT)
			{
				mServo->setRev(REVERSED);
				SendMessage(GetDlgItem(parentDlg, IDC_STATUS), SB_SETTEXT, 3, (LPARAM)"Reversed: Yes");
				mServo->reverse = REVERSED;
			}
			else
			{
				mServo->setRev(DEFAULT);
				SendMessage(GetDlgItem(parentDlg, IDC_STATUS), SB_SETTEXT, 3, (LPARAM)"Reversed: No");
				mServo->reverse = DEFAULT;
			}
			mServo->testRun();
			EndDialog(hDlg, IDOK);
			return TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			//cancle
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hDlg);
		return TRUE;

	case WM_DESTROY:
		EndDialog(hDlg, IDCANCEL);
		return TRUE;
	}
	return FALSE;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE h0, LPTSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	BOOL ret;
	HWND hDlg;

	hInst = hInstance;
	mServo = new FutabaServo();

	InitCommonControls();
	hDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAIN), 0, MainProc);
	ShowWindow(hDlg, nCmdShow);

	//set default ID number, disable buttons before COM connection
	SendMessage(GetDlgItem(hDlg, IDC_RPU), BM_SETCHECK, BST_CHECKED, 0);
	EnableWindow(GetDlgItem(hDlg, IDC_READ), false);
	EnableWindow(GetDlgItem(hDlg, IDC_RESET), false);
	EnableWindow(GetDlgItem(hDlg, IDC_CHANGE), false);
	EnableWindow(GetDlgItem(hDlg, IDC_REVERSE), false);
	EnableWindow(GetDlgItem(hDlg, IDC_ZERO), false);

	sliderInit(hDlg);
	createStatusBar(hDlg);
	EnableWindow(GetDlgItem(hDlg, IDC_SLIDER1), false);

	//look for available COM port, add to combo box
	std::string comList[256];
	int comNum = mServo->mPort->look4Port(comList);
	HWND hCombCOM = GetDlgItem(hDlg, IDC_COMBOCOM);
	for (int i = 0; i < comNum; i++)
	{
		SendMessage(hCombCOM, CB_ADDSTRING, 0, (LPARAM)(comList[i].c_str()));
	}
	SendMessage(hCombCOM, CB_SETCURSEL, 0, 0);
	
	while ((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
		if (ret == -1)
			return -1;

		if (!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}