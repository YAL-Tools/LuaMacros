#pragma once
#include "stdafx.h"
// Allows to convert between wchar_t and utf-8 char strings.
// Make sure to make a copy of what you get back!
struct StringConvImpl {
	LPCWSTR proc(LPCSTR str);
	LPCSTR proc(LPCWSTR str);
};
extern StringConvImpl StringConv;