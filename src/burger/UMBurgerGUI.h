/**
 * @file UMBurgerGUI.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include "UMMacro.h"
#include <memory>
#include "UMGUIScene.h"

namespace burger
{

class UMBurgerGUI;
typedef std::shared_ptr<UMBurgerGUI> UMBurgerGUIPtr;

class UMBurgerGUI : public umgui::UMGUIScene
{
	DISALLOW_COPY_AND_ASSIGN(UMBurgerGUI);
public:
	UMBurgerGUI() {}
	virtual ~UMBurgerGUI() {}
	
	/**
	 * initialize gui components
	 */
	virtual bool init(int width, int height);

};

} // burger
