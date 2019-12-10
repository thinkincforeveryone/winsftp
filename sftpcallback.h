#ifndef _SFTPCALLBACK_H_
#define _SFTPCALLBACK_H_
#include <stdbool.h>
#include <Windows.h>

HWND g_hWnd;

HWND sftp_win_create(HWND parent, HINSTANCE hInstance);
BOOL sftp_win_resize(HWND hWnd, const RECT* rt); 
BOOL sftp_win_set_pos(HWND hWnd, int x, int y, int width, int height);
void sftp_win_set_visible(HWND hWnd, BOOL bVisible); 
void sftp_win_term_paint(HWND hWnd);

void add_line_text(char *text); 

 DWORD  WINAPI thead_do_sftp( LPVOID lpThreadParameter );
 int start_sftp();
 int psft_printf(char const* const _Format, ...);
 int do_loop(HWND hWnd);
 void do_key_input(WPARAM wParam, LPARAM lParam);



typedef struct tagWINTERM
{
	HWND(*create)(HWND, HINSTANCE);
	BOOL(*resize)(HWND,const RECT*);
	BOOL(*set_pos)(HWND, int, int, int, int);
	void(*set_visible)(HWND, BOOL); 
	void(*term_paint)(HWND);
	HWND hWnd; 
}WINTERM;



#endif
