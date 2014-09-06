/**
 * @file UMMain.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#if !defined(WITH_EMSCRIPTEN)
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#endif
#endif

//struct CrtBreakAllocSetter {
//    CrtBreakAllocSetter() {
//        _crtBreakAlloc=26434;
//    }
//};
//CrtBreakAllocSetter g_crtBreakAllocSetter;

#include "UMWindow.h"

// main
int main(int argc, char** argv)
{
#if !defined(WITH_EMSCRIPTEN)
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(27125);

#endif
	return burger::UMWindow::instance().main(argc, argv);
}
