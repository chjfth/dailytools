#ifndef __easyclipboard_h_20150914_
#define __easyclipboard_h_20150914_

#ifdef __cplusplus
extern"C"{
#endif


BOOL easySetClipboardText(const TCHAR Text[], int textchars=-1, HWND hwnd=0);
/*
	Put Text to clipboard. 
	textchars tell how many characters to put, -1 means count by lstrlen.
*/

TCHAR *easyGetClipboardText(int *ptextchars=0, HWND hwnd=0);
	// Not tested yet in Keyview2.
/*
	Get text from clipboard.
	Return value points to the text(a copy), NUL-terminated.
	You should free this text buffer later by calling  easyFreeClipboardText().
	Returning NULL means no text in clipboard currently.

	On return, *ptextchars tells the returned string length, in characters,
	not counting terminating NUL.
*/

BOOL easyFreeClipboardText(TCHAR *p);


#ifdef __cplusplus
} // extern"C"{
#endif

#endif
