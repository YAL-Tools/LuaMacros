#include "stdafx.h"
#include "StringConv.h"
#include <string>
StringConvImpl StringConv;

LPCWSTR StringConvImpl::proc(LPCSTR str) {
    if (str == NULL) return NULL;
    auto len = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    static std::wstring wide{};
    wide.resize(len);
    MultiByteToWideChar(CP_UTF8, 0, str, -1, (wchar_t*)wide.data(), len);
    return wide.c_str();
}
LPCSTR StringConvImpl::proc(LPCWSTR str) {
    if (str == NULL) return NULL;
    auto len = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    static std::string utf8{};
    utf8.resize(len);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, (char*)utf8.data(), len, NULL, NULL);
    return utf8.c_str();
}