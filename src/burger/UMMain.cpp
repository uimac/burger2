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
	#include <crtdbg.h>
#endif

#include "UMWindow.h"

// main
int main(int argc, char** argv)
{
#if !defined(WITH_EMSCRIPTEN)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	return test_viewer::UMWindow::instance().main(argc, argv);
}
