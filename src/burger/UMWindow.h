/**
 * @file UMWindow.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#if !defined(WITH_EMSCRIPTEN)
	#include <tchar.h>
	#include <windows.h>
#endif 

#include <memory>
#include "UMMacro.h"

namespace test_viewer
{

/**
 * main window
 */
class UMWindow
{
	DISALLOW_COPY_AND_ASSIGN(UMWindow);

public:

	/**
	 * get singleton instance
	 */
	static UMWindow& instance() {
		static UMWindow window;
		return window;
	}
	
	/**
	 * just a wrapper of main
	 */
	int main(int argc, char** argv);

private:
	UMWindow() : width_(1024), height_(1024) {}

	/// initial client width
	int width_;
	/// initial client height
	int height_;
};

} // test_viewer
