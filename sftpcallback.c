#include "sftpcallback.h"

const LPCSTR WINTAB_SFTP_PAGE_CLASS = "WintabSftpPage";

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int cxChar, cxCaps, cyChar, cxClient, cyClient, iMaxWidth;
	HDC hdc;
	int i, x, y, iVertPos, iHorzPos, iPaintBeg, iPaintEnd;
	PAINTSTRUCT ps;
	SCROLLINFO si;
	TCHAR szBuffer[10];
	TEXTMETRIC tm;
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
		si.nMax = si.nPage * 2;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = 2 + iMaxWidth / cxChar;
		si.nPage = cxClient / cxChar;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
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

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetScrollInfo(hWnd, SB_VERT, &si);
		iVertPos = si.nPos;
		GetScrollInfo(hWnd, SB_HORZ, &si);
		iHorzPos = si.nPos;
		iPaintBeg = max(0, iVertPos + ps.rcPaint.top / cyChar);
		iPaintEnd = min(si.nPage,
		iVertPos + ps.rcPaint.bottom / cyChar);
		HBRUSH br = CreateSolidBrush(0x00000000);
		SelectObject(hdc, br);

		SetBkMode(hdc, TRANSPARENT);
		//SetBkColor(hdc, 0x00000000);
		SetTextColor(hdc, 0x000000ff);
		//SetCursor(SelectObject(IDC_CI_CURRENT));
		
		for (i = iPaintBeg; i <= iPaintEnd; i++)
		{
			x = 20;
			y = cyChar * (i - iVertPos); 
			
			TextOut(hdc, x, y, "kalsdjfalfj", strlen("kalsdjfalfj"));
			SetTextAlign(hdc, TA_LEFT | TA_TOP);
		}
		IntersectClipRect(hdc, 20, 0, cxCaps * strlen("kalsdjfalfj"), cyChar * 45);
		ExcludeClipRect(hdc, 20, 0, cxCaps * strlen("kalsdjfalfj"), cyChar * 45);
		Rectangle(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);
		DeleteObject(br); 
		EndPaint(hWnd, &ps);
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
	
	return hWnd;
}

BOOL sftp_win_resize(HWND hWnd, const RECT* rt)
{
	return MoveWindow(hWnd, rt->left, rt->top, rt->right - rt->left, 
		rt->bottom - rt->top, false);
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
	PAINTSTRUCT p;
	HDC hdc= BeginPaint(hWnd, &p);
	//std::string str = "ABBBBBBBBBBBBBBBBBBDDDDDDDDDDDEEEEEEEEE";
	TextOut(hdc, 50, 50, "ABBBBBBBBBBBBBBBBBBDDDDDDDDDDDEEEEEEEEE", 
		strlen("ABBBBBBBBBBBBBBBBBBDDDDDDDDDDDEEEEEEEEE")); 
	IntersectClipRect(hdc,
		p.rcPaint.left, p.rcPaint.top,
		p.rcPaint.right, p.rcPaint.bottom);
	ExcludeClipRect(hdc, 50, 50, 300, 70);
	Rectangle(hdc, p.rcPaint.left, p.rcPaint.top, p.rcPaint.right, p.rcPaint.bottom);
	EndPaint(hWnd, &p);
}
