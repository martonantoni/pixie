#include "StdAfx.h"

CString Format(const char* lpszFormat,...)
{
	CString s;
	va_list argList;
	va_start(argList, lpszFormat);
	s.FormatV(lpszFormat, argList);
	va_end(argList);
	return s;
}
