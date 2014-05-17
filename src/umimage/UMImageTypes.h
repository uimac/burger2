/**
 * @file UMImageTypes.h
 * Image types
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

// caution! this header defines global typedefs.

#include <memory>

namespace umimage
{

class UMImage;
typedef std::shared_ptr<UMImage> UMImagePtr;

};

typedef umimage::UMImage UMImage;
typedef umimage::UMImagePtr UMImagePtr;
