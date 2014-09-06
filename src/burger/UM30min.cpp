/**
 * @file UM30min.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UM30min.h"
#include "UMTime.h"
#include "UMTga.h"
#include "UMPath.h"
#include "UMStringUtil.h"
#include "UMViewer.h"

namespace burger
{

	static int number_offset  = 1;

/**
 * constructor
 */
UM30min::UM30min(UMViewerPtr viewer)
{
	viewer_ = viewer;
	current_number_ = 0 + number_offset;
	initial_time_ = umbase::UMTime::current_time();
}

UM30min::~UM30min()
{
}

/**
 * return true on update, check time
 * or return false on exit
 */
bool UM30min::update()
{
	// milliseconds
	unsigned long time = umbase::UMTime::current_time() - initial_time_;
	unsigned long minutes = time / 60000;
	unsigned long seconds = (time - minutes * 60) / 1000;

	if (minutes >= current_number_ - number_offset)
	//if (minutes >= 0 && seconds > 10)
	{
		if (UMViewerPtr viewer = viewer_.lock())
		{
			//viewer->change_to_abc_camera();
			viewer->set_current_frame(current_number_);

			UMImagePtr image = viewer->render();
			image = image->create_flip_image(false, true);

			std::string number = umbase::UMStringUtil::number_to_sequence_string(current_number_ - number_offset, 4);
			std::string filename = "\\" + number + ".png";
			umstring utf16name = umbase::UMStringUtil::utf8_to_utf16(filename);
		
			umstring outpath = umbase::UMPath::get_base_folder(umbase::UMPath::module_absolute_path());
			outpath = outpath.append(utf16name);

			umimage::UMImage::save(outpath, image, umimage::UMImage::eImageTypePNG_RGBA);
		}
		++current_number_;
	}

	if (current_number_ >= 3 + number_offset)
	{
		printf("------------------END-----------------\n");
		return false;
	}
	return true;
}

} //burger
