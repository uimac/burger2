/**
 * @file UMPath.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#if !defined(WITH_EMSCRIPTEN)
#include <windows.h>
#include <Mmsystem.h>
#include <tchar.h>
#include <shlwapi.h>
#endif 

#include <string>

#include "UMPath.h"
#include "UMStringUtil.h"

namespace umbase
{
	
bool UMPath::exists(const umstring& absolute_path)
{
#ifdef WITH_EMSCRIPTEN
	return true;
#else
	std::wstring inpath = UMStringUtil::utf16_to_wstring(absolute_path);
	if (::PathFileExistsW(inpath.c_str()))
	{
		return true;
	}
	return false;
#endif // WITH_EMSCRIPTEN
}

umstring UMPath::module_absolute_path()
{
#ifdef WITH_EMSCRIPTEN
	umstring none;
	return none;
#else
	TCHAR path[1024];
	GetModuleFileName(NULL, path, sizeof(path) / sizeof(TCHAR));
	return UMStringUtil::wstring_to_utf16(path);
#endif // WITH_EMSCRIPTEN
}

umstring UMPath::resource_absolute_path(const umstring& file_name)
{
#ifdef WITH_EMSCRIPTEN
	return umstring("resource/") + file_name;
#else
	TCHAR path[1024];
	GetModuleFileName(NULL, path, sizeof(path) / sizeof(TCHAR));
	PathRemoveFileSpec(path);
	SetCurrentDirectory(path);
	SetCurrentDirectory(_T("../../../resource/"));
	GetCurrentDirectory(1024, path);
	std::wstring inpath = path + std::wstring(_T("\\")) + UMStringUtil::utf16_to_wstring(file_name);
	// honban you kozaiku
	if (! ::PathFileExistsW(inpath.c_str()))
	{
		SetCurrentDirectory(path);
		SetCurrentDirectory(_T("./resource/"));
		GetCurrentDirectory(1024, path);
		inpath = path + std::wstring(_T("\\")) + UMStringUtil::utf16_to_wstring(file_name);
	}
	return UMStringUtil::wstring_to_utf16(inpath);
#endif // WITH_EMSCRIPTEN
}

umstring UMPath::get_file_name(const umstring& file_path)
{
#ifdef WITH_EMSCRIPTEN
	return file_path;
#else
	std::wstring path = UMStringUtil::utf16_to_wstring(file_path);
	std::wstring filename(::PathFindFileName(path.c_str()));
	return UMStringUtil::wstring_to_utf16(filename);
#endif // WITH_EMSCRIPTEN
}

umstring UMPath::get_absolute_path(const umstring& base_path, umstring& file_name)
{
#ifdef WITH_EMSCRIPTEN
	return base_path + "/" + file_name;
#else
	umstring base_path_copy = base_path;
	umstring base_file_name = get_file_name(base_path_copy);
	if (!base_file_name.empty())
	{
		umstring::size_type pos = base_path_copy.find(base_file_name);
		if (pos != umstring::npos)
		{
			base_path_copy.erase(
				base_path_copy.begin() + pos ,
				base_path_copy.begin() + pos + base_file_name.length());
		}
	}
	return base_path_copy + file_name;
#endif // WITH_EMSCRIPTEN
}

umstring UMPath::get_env(const umstring& env)
{
#ifdef WITH_EMSCRIPTEN
	return env;
#else
	if (wchar_t* val =  _wgetenv(UMStringUtil::utf16_to_wstring(env).c_str()))
	{
		return UMStringUtil::wstring_to_utf16(val);
	}
	umstring none;
	return none;
#endif // WITH_EMSCRIPTEN
}

} // umbase

