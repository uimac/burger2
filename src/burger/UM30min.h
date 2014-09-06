/**
 * @file UM30min.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include "UMMacro.h"

namespace burger
{
	
class UMViewer;
typedef std::shared_ptr<UMViewer> UMViewerPtr;
typedef std::weak_ptr<UMViewer> UMViewerWeakPtr;

class UM30min;
typedef std::shared_ptr<UM30min> UM30minPtr;

class UM30min
{
public:
	
	/**
	 * constructor
	 */
	UM30min(UMViewerPtr viewer);

	~UM30min();

	bool update();
	
private:
	UMViewerWeakPtr viewer_;
	unsigned long initial_time_;
	unsigned int current_number_;
};

} // burger