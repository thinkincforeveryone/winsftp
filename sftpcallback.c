#include "sftpcallback.h"
#include "puttymem.h"
#include <stdio.h>
#include "psftp.h" 

#define MAX_LINE 1000
#define MAX_ONE_LINE_CHAR 1024
#define ARGV_P "-P"
#define ARGV_L "-l"
#define ARGV_PW "-pw"
#define ARGV_SSHLOGFILE "-sshlog"

const LPCSTR WINTAB_SFTP_PAGE_CLASS = "WintabSftpPage";
static int cxChar, cxCaps, cyChar, cxClient, cyClient, iMaxWidth;
int i, x, y, iVertPos, iHorzPos, iPaintBeg, iPaintEnd;
HDC hdc;
PAINTSTRUCT ps;
SCROLLINFO si;
TCHAR szBuffer[10];
TEXTMETRIC tm;
char *lines[MAX_LINE];
unsigned int last_line = 0;


void initLines()
{
	for (int i = 0; i < MAX_LINE - 1; i++)
	{
		lines[i] = snewn(MAX_ONE_LINE_CHAR, char);
		memset(lines[i], 0, MAX_ONE_LINE_CHAR);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{  
	switch (message)
	{
	case WM_CREATE:
		hdc = GetDC(hWnd);
		GetTextMetrics(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) *cxChar / 2;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		ReleaseDC(hWnd, hdc);
		iMaxWidth = 40 * cxChar + 22 * cxCaps;
		return 0;
	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;

		si.nPage = cyClient / cyChar;
		si.nMax = MAX_LINE - 1;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE); 
		return 0;
	case WM_VSCROLL:
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hWnd, SB_VERT, &si);
		iVertPos = si.nPos;
		switch (LOWORD(wParam))
		{
		case SB_TOP:
			si.nPos = si.nMin;
			break;
		case SB_BOTTOM:
			si.nPos = si.nMax;
			break;
		case SB_LINEUP:
			si.nPos -= 1;
			break;
		case SB_LINEDOWN:
			si.nPos += 1;
			break;
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;
		default:
			break;
		}
		si.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hWnd, SB_VERT, &si);
		if (si.nPos != iVertPos)
		{
			ScrollWindow(hWnd, 0, cyChar * (iVertPos - si.nPos),
				NULL, NULL);
			UpdateWindow(hWnd);
		}
		return 0; 
	case WM_CHAR:  
		do_key_input(wParam, lParam);
		break;

	case WM_PAINT:
		sftp_win_term_paint(hWnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
 
BOOL register_class(HINSTANCE hinst)
{
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hinst;
	wndclass.hIcon = NULL;
	wndclass.hCursor = NULL;
	wndclass.hbrBackground = CreateSolidBrush(0x00000000);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = WINTAB_SFTP_PAGE_CLASS;
	wndclass.hIconSm = 0;
	return RegisterClassEx(&wndclass);
}

HWND sftp_win_create(HWND parent, HINSTANCE hInstance)
{
	initLines();
	register_class(hInstance);
	int winmode = WS_VSCROLL;
	if (parent)
	{
		winmode = winmode | WS_CHILD;
	}
	HWND hWnd = CreateWindowEx(
		NULL,
		WINTAB_SFTP_PAGE_CLASS,
		WINTAB_SFTP_PAGE_CLASS,
		WS_OVERLAPPEDWINDOW,
		0, 0, 0, 0,
		parent,
		NULL,   /* hMenu */
		hInstance,
		NULL);  /* lpParam */
	g_hWnd = hWnd;
	return hWnd;
}

BOOL sftp_win_resize(HWND hWnd, const RECT* rt)
{
	return MoveWindow(hWnd, rt->left, rt->top, rt->right - rt->left, 
		rt->bottom - rt->top, true);
}

BOOL sftp_win_set_pos(HWND hWnd, int x, int y, int width, int height)
{ 
	return SetWindowPos(hWnd, NULL, x, y, width, height, 0);
}

void sftp_win_set_visible(HWND hWnd, bool bVisible)
{
	if( bVisible )
	{
		ShowWindow(hWnd, SW_SHOW);
	}
	else 
	{
		ShowWindow(hWnd, SW_HIDE);
	}
}

HDC sftp_win_get_ctx(HWND hWnd)
{
	return 	GetDC(hWnd);
}

int sftp_win_free_ctx(HWND hWnd, const HDC hdc)
{
	return ReleaseDC(hWnd, hdc);
}

void sftp_win_term_paint(HWND hWnd)
{
	hdc = BeginPaint(hWnd, &ps);
	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	GetScrollInfo(hWnd, SB_VERT, &si);
	iVertPos = si.nPos;
	iPaintBeg = max(0, iVertPos + ps.rcPaint.top / cyChar);
	iPaintEnd = min(MAX_LINE - 1,
		iVertPos + ps.rcPaint.bottom / cyChar);
	HBRUSH br = CreateSolidBrush(0x00000000);
	HBRUSH oldBr = (HBRUSH) SelectObject(hdc, br);

	SetBkMode(hdc, TRANSPARENT);
	//SetBkColor(hdc, 0x00000000);
	SetTextColor(hdc, 0x00ffff00);
	//SetCursor(SelectObject(IDC_CI_CURRENT));
	int len = 0;
	for (i = iPaintBeg; i <= iPaintEnd; i++)
	{
		x = 20;
		y = cyChar * (i - iVertPos);
		len = strlen(lines[i]);
		if (len > 0)
		{ 
			TextOut(hdc, x, y, lines[i], len);
			SetTextAlign(hdc, TA_LEFT | TA_TOP);
		}
		
	}
	IntersectClipRect(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
	ExcludeClipRect(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
	Rectangle(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);

	SelectObject(hdc, oldBr);
	DeleteObject(br);
	EndPaint(hWnd, &ps);
}

void add_line_text(char *text)
{
	int pos = strcspn(text, "\n");
	text[pos] = '\0';
	if ( (last_line < MAX_LINE - 1) 
		&& (strlen(text) < MAX_ONE_LINE_CHAR)
		)
	{
		strcpy(lines[++last_line], text); 
		//PostMessage(g_hWnd, WM_PAINT, NULL, NULL);
		//RedrawWindow(g_hWnd, NULL, NULL, RDW_INTERNALPAINT); 


		si.nPos = last_line - cyClient / cyChar + 1;
		si.fMask = SIF_POS;
		SetScrollInfo(g_hWnd, SB_VERT, &si, TRUE); 
		ScrollWindow(g_hWnd, 0, cyChar * si.nPos, NULL, NULL);  


		//InvalidateRect(g_hWnd, NULL, false);
	}
	
}

DWORD WINAPI thead_do_sftp(LPVOID lpThreadParameter)
{
	WINTERM *pwinterm = (WINTERM *)lpThreadParameter;
	if (pwinterm)
	{
		start_sftp();
	}
	
}

int start_sftp()
{

	int argc = 8;
	char *argv[8];
	for (int i = 0; i < argc; i++)
	{
		argv[i] = snewn(256, char);
		memset(argv[i], 0, 256);
	}
	char *name = "fyx";
	char *host = "192.168.128.222";
	int port = 22;
	char *pass = "fyx999";

	memcpy(argv[1], ARGV_P, strlen(ARGV_P));
	itoa(port, argv[2], 10);
	memcpy(argv[3], ARGV_L, strlen(ARGV_L));
	memcpy(argv[4], name, strlen(name));
	memcpy(argv[5], ARGV_PW, strlen(ARGV_PW));
	memcpy(argv[6], pass, strlen(pass));
	memcpy(argv[7], host, strlen(host));

	return psftp_main(argc, argv);
}

int psft_printf(char const* const format, ...)
{
	char buffer[256] = { 0 };
	int result = 0;
	va_list args;
	va_start(args, format);
	result = vsprintf(buffer, format, args); 
	va_end(args);
	if (result > 0)
	{
		add_line_text(buffer);
	}
	return result;
}

int do_loop(HWND hWnd)
{
	MSG msg;
	int ret;
	while (ret = GetMessage(&msg, hWnd, 0, 0))
	{
		if (ret <= -1) // error
		{
			return ret;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return msg.wParam;
}

void do_key_input(WPARAM wParam, LPARAM lParam)
{
	static char strcmd[512] = { 0 };
	static int pos = 0;
	if (wParam == 13)
	{
		//´¦Àí
		int ret;
		struct sftp_command *cmd;
		cmd = sftp_getcmd(strcmd, 0, 0, 0);
		ret = cmd->obey(cmd);
		add_line_text("psftp>");
		
		//Çå³ý
		memset(strcmd, 0, 512);
		pos = 0;
	}
	else
	{
		strcmd[pos++] = wParam;
		int len = strlen(lines[last_line]);
		*(char*)(lines[last_line]+ len) = wParam;
		InvalidateRect(g_hWnd, NULL, false);
	}
}


INT WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	int ret;

	dll_hijacking_protection();

	//ret = psftp_main(argc, argv);
	WINTERM   WinTerm = {
	sftp_win_create,
	sftp_win_resize,
	sftp_win_set_pos,
	sftp_win_set_visible,
	sftp_win_term_paint,
	NULL
	};


	HWND hWnd = WinTerm.create(NULL, hInstance);
	WinTerm.set_visible(hWnd, true);
	RECT rt = { 0 };
	GetWindowRect(GetDesktopWindow(), &rt);
	WinTerm.resize(hWnd, &rt);
	//WinTerm.set_pos(hWnd, 0, 0, rt.right - rt.left, rt.bottom - rt.top);

	DWORD threadid;
	//CreateThread(0, 0, thead_do_sftp, &WinTerm, 0, &threadid);


	//MSG msg; 
	//while ( ret = GetMessage(&msg, NULL, 0, 0) )
	//{
	//	TranslateMessage(&msg);
	//	DispatchMessage(&msg);
	//}

	return start_sftp();
}

