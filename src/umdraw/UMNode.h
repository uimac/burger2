/**
 * @file UMNode.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include "UMMacro.h"
#include "UMMathTypes.h"
#include "UMVector.h"
#include "UMMatrix.h"
#include "UMShaderEntry.h"

namespace umdraw
{

class UMNode;
typedef std::shared_ptr<UMNode> UMNodePtr;
typedef std::vector<UMNodePtr> UMNodeList;

/**
 * a node
 */
class UMNode
{
	DISALLOW_COPY_AND_ASSIGN(UMNode);
public:
	UMNode() {
		static unsigned int counter = 0;
		id_ = ++counter;
	}
	~UMNode() {}
	
	// getter
	unsigned int id() const { return id_; }
	const std::u16string& name() const { return name_; }
	const UMMat44d& local_transform() const { return local_transform_; }
	const UMMat44d& global_transform() const { return global_transform_; }

	bool is_valid_shader_entry() const { return shader_entry_ ? true : false; }
	UMShaderEntryPtr shader_entry() const { return shader_entry_; }

	// setter
	void set_name(const std::u16string& name) { name_ = name; }
	UMMat44d& mutable_local_transform() { return local_transform_; }
	UMMat44d& mutable_global_transform() { return global_transform_; }

	void set_shader_entry(UMShaderEntryPtr shader_entry) { shader_entry_ = shader_entry; }

private:
	unsigned int id_;
	std::u16string name_;

	// evaluated transform
	UMMat44d local_transform_;
	UMMat44d global_transform_;

	UMShaderEntryPtr shader_entry_;
};

}
