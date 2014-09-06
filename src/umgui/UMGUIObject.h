/**
 * @file UMGUIObject.h
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include <string>

#include "UMMacro.h"
#include "UMBox.h"
#include "UMMathTypes.h"
#include "UMNode.h"
#include "UMGUI.h"
#include "UMEvent.h"


namespace umdraw
{
class UMMesh;
typedef std::shared_ptr<UMMesh> UMMeshPtr;
}

namespace umimage
{
class UMImage;
typedef std::shared_ptr<UMImage> UMImagePtr;
}

namespace umgui
{

class UMGUIObject;
typedef std::shared_ptr<UMGUIObject> UMGUIObjectPtr;
typedef std::weak_ptr<UMGUIObject> UMGUIObjectWeakPtr;
typedef std::vector<UMGUIObjectPtr> UMGUIObjectList;

class UMGUIObject : public umdraw::UMNode
{
	DISALLOW_COPY_AND_ASSIGN(UMGUIObject);
public:
	UMGUIObject();

	virtual ~UMGUIObject();
	
	/**
	 * initialize
	 * @param [in] recursive do children recursively
	 */
	virtual bool init(bool recursive);
	
	/**
	 * is valid or not
	 */
	virtual bool is_valid() const { return true; }

	/**
	 * is visible or not
	 */
	virtual bool is_visible() const;

	/**
	 * set visible
	 */
	virtual void set_visible(bool is_visible) { is_visible_ = is_visible; }
	
	/**
	 * root board or not
	 */
	virtual bool is_root() const { return is_root_; }

	/**
	 * node or not
	 */
	virtual bool is_node() const { return is_node_; }

	/**
	 * get bounding box
	 */
	virtual const UMBox& box() const { return box_; }
	
	/**
	 * get initial bounding box
	 */
	virtual const UMBox& initial_box() const { return initial_box_; }

	/**
	 * update box
	 * @param [in] recursive do children recursively
	 */
	virtual void update_box(bool recursive);

	/**
	 * draw
	 */
	virtual void draw(bool recursive, UMGUI::DrawType type);
	
	/**
	 * update
	 */
	virtual bool update(bool recursive);
	
	/**
	 * intersect
	 */
	static void intersect(UMGUIObjectPtr object, UMGUIObjectList& intersect_list, double x, double y);
	
	/**
	 * intersect
	 */
	virtual bool intersect(double x, double y) const;
	
	/**
	 * get mesh
	 */
	virtual umdraw::UMMeshPtr mesh() { return umdraw::UMMeshPtr(); }

	/**
	 * get children
	 */
	const UMGUIObjectList& children() const { return children_; }
	
	/**
	 * get children
	 */
	UMGUIObjectList& mutable_children() { return children_; }

	/**
	 * get name
	 */
	const std::string& name() const { return name_; }

	/**
	 * get update event
	 */
	umbase::UMEventPtr update_event() { return update_event_; }

protected:
	virtual void on_left_button_down(double x, double y) {}
	virtual void on_left_button_move(double x, double y) {}
	virtual void on_left_button_up(double x, double y) {}
	virtual void on_scroll(double x, double y) {}

	std::string name_;
	UMBox box_;
	UMBox initial_box_;
	UMGUIObjectList children_;
	bool is_visible_;
	bool is_root_;
	bool is_node_;
	umbase::UMEventPtr update_event_;

	friend UMGUIScene;
};

} // umgui
