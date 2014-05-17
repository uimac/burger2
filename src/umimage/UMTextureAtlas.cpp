/**
 * @file UMTextureAtlas.cpp
 * texture atlas
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMTextureAtlas.h"
#include <map>
#include "UMVector.h"
#include "UMImage.h"

namespace umimage
{

namespace
{
	class UMTextureAtlasNode;
	typedef std::shared_ptr<UMTextureAtlasNode> AtlasNodePtr;

	class UMTextureAtlasNode
	{
	public:
		UMTextureAtlasNode()
			: left(AtlasNodePtr())
			, right(AtlasNodePtr())
			, rect(0)
			, is_assigned(false)
		{}
		UMTextureAtlasNode(int width, int height)
			: left(AtlasNodePtr())
			, right(AtlasNodePtr())
			, rect(0, 0, width, height)
			, is_assigned(false)
		{}
		AtlasNodePtr left;
		AtlasNodePtr right;
		umbase::UMVec4ui rect;
		bool is_assigned;
	};

	int rect_width(const umbase::UMVec4ui& rect)
	{
		return rect[2]-rect[0];
	}

	int rect_height(const umbase::UMVec4ui& rect)
	{
		return rect[3]-rect[1];
	}
	
	AtlasNodePtr insert_image(UMImagePtr image, AtlasNodePtr node)
	{
		if (node->left || node->right)
		{
			if (AtlasNodePtr new_node = insert_image(image, node->left))
			{
				return new_node;
			}
			return insert_image(image, node->right);
		}
		else
		{
			if (node->is_assigned)
			{
				// there is already rect
				return AtlasNodePtr();
			}
			if (image->width() > rect_width(node->rect) || image->height() > rect_height(node->rect)) 
			{
				// not fit
				return AtlasNodePtr();
			}
			if (image->width() == rect_width(node->rect) && image->height() == rect_height(node->rect))
			{
				// fit
				return node;
			}

			node->left = std::make_shared<UMTextureAtlasNode>();
			node->right = std::make_shared<UMTextureAtlasNode>();
			const int image_width = image->width();
			const int image_height = image->height();
			const int dw = rect_width(node->rect) - image->width();
			const int dh = rect_height(node->rect) - image->height();
			if (dw > dh)
			{
				// split horizontal
				node->left->rect = UMVec4ui(node->rect[0], node->rect[1], node->rect[0]+image_width, node->rect[3]);
				node->right->rect = UMVec4ui(node->rect[0]+image_width+1, node->rect[1], node->rect[2], node->rect[3]);
			}
			else
			{
				// split vertical
				node->left->rect = UMVec4ui(node->rect[0], node->rect[1], node->rect[2], node->rect[1]+image_height);
				node->right->rect = UMVec4ui(node->rect[0], node->rect[1]+image_height+1, node->rect[2], node->rect[3]);
			}
			return insert_image(image, node->left);
		}
	}
}

class UMTextureAtlas::AtlasImpl
{
	DISALLOW_COPY_AND_ASSIGN(AtlasImpl);
public:
	AtlasImpl(int width, int height)
		: width_(width)
		, height_(height)
		, atlas_image_(std::make_shared<UMImage>())
		, root_(new UMTextureAtlasNode(width, height))
	{
		atlas_image_->init(width, height);
	}

	~AtlasImpl() {}

	UMImagePtr atlas_image() { return atlas_image_; }
	void set_width(int width) { width_ = width; }
	void set_height(int height) { height_ = height; }

	bool add_text_image(UMImagePtr image, const char16_t& text)
	{
		if (!image) return false;
		if (image->width() > width_) return false;
		if (image->height() > height_) return false;

		AtlasNodePtr node = insert_image(image, root_);
		if (node)
		{
			rect_map_[text] = node->rect;
			image->copy(atlas_image_, node->rect);
			node->is_assigned = true;
			return true;
		}
		return false;
	}
	
	bool is_exist(const char16_t& text) const
	{
		RectMap::const_iterator it = rect_map_.find(text);
		if (it != rect_map_.end())
		{
			return true;
		}
		return false;
	}

	umbase::UMVec4ui text_rect(const char16_t& text) const
	{
		RectMap::const_iterator it = rect_map_.find(text);
		if (it != rect_map_.end())
		{
			return it->second;
		}
		return umbase::UMVec4ui(0);
	}

private:
	int width_;
	int height_;
	UMImagePtr atlas_image_;
	typedef std::map<char16_t, umbase::UMVec4ui> RectMap;
	RectMap rect_map_;

	AtlasNodePtr root_;
};

/**
 * constructor
 */
UMTextureAtlas::UMTextureAtlas(int width, int height)
	: impl_(new UMTextureAtlas::AtlasImpl(width, height))
{
}

/**
 * destructor
 */
UMTextureAtlas::~UMTextureAtlas()
{
}

/**
 * get atlas
 */
UMImagePtr UMTextureAtlas::atlas_image()
{
	return impl_->atlas_image();
}

/**
 * add text image to atlas
 */
bool UMTextureAtlas::add_text_image(UMImagePtr image, const char16_t& text)
{
	return impl_->add_text_image(image, text);
}

/**
 * get text bounds
 */
umbase::UMVec4ui UMTextureAtlas::text_rect(const char16_t& text) const
{
	return impl_->text_rect(text);
}

/**
 * text exists or not
 */
bool UMTextureAtlas::is_exist(const char16_t& text) const
{
	return impl_->is_exist(text);
}

} // umimage
