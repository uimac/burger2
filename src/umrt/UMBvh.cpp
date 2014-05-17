/**
 * @file UMBvh.cpp
 * bounding volume hierarchy
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMBvh.h"
#include <algorithm>
#include <assert.h>
#include "UMMathTypes.h"
#include "UMMath.h"
#include "UMBox.h"
#include "UMRay.h"

namespace umrt
{

/**s
 * bvh node
 */
class UMBvhNode
{
public:
	UMBvhNode()
		: axis_(0),
		start_index_(0),
		end_index_(0),
		flat_index_(0)
	{}

	void init_as_leaf(const umbase::UMBox& box, int start_index, int end_index)
	{
		box_.set_minimum(box.minimum());
		box_.set_maximum(box.maximum());
		start_index_ = start_index;
		end_index_ = end_index;
	}

	void init_as_branch(UMBvhNodePtr left, UMBvhNodePtr right, int axis)
	{
		axis_ = axis;
		start_index_ = 0;
		end_index_ = 0;
		left_ = left;
		right_ = right;
		box_.init();
		box_.extend(left->box_);
		box_.extend(right->box_);
	}

	bool is_leaf() const { return end_index_ > start_index_; }

	umbase::UMBox box_;
	UMBvhNodePtr left_;
	UMBvhNodePtr right_;
	unsigned char axis_;
	int start_index_;
	int end_index_;
	// ordered index in flatten
	int flat_index_;
};

}// umstructure

namespace
{
	using namespace umrt;
	using namespace umdraw;
	using namespace umbase;

	/**
	 * SAH const function
	 * @param [in] area area of target prims' AABB
	 * @param [in] parted_area1 area1 of parted prims' AABB
	 * @param [in] parted_primitive_count1 parted primitive counts
	 * @param [in] parted_area1 area2 of parted prims' AABB
	 * @param [in] parted_primitive_count2 parted primitive counts
	 * @retval cost
	 */
	double sah(
		double area,
		double parted_area1,
		unsigned int parted_primitive_count1,
		double parted_area2, 
		unsigned int parted_primitive_count2)
	{
		double inv_area = 1.0 / area;
		return 0.2 // value from pbrt
			+ ((parted_area1 * parted_primitive_count1)
			+  (parted_area2 * parted_primitive_count2)) * inv_area;
	}

	/**
	 * primitive comparator
	 */
	struct before_middle {
		before_middle(int axis_, double middle_)
			: axis(axis_),
			middle(middle_)
		{}
		int axis;
		double middle;
		bool operator() (const UMPrimitivePtr a) const {
			return a->box().center()[axis] < middle;
		}
	};

	/**
	 * primitive comparator
	 */
	struct before_less {
		before_less(int axis_)
			: axis(axis_)
		{}
		int axis;
		bool operator() (const UMPrimitivePtr a, const UMPrimitivePtr b) const {
			return a->box().center()[axis] < b->box().center()[axis];
		}
	};
	
	struct compare_bucket {
		compare_bucket(int split, int num, int axis_, const umbase::UMBox &b)
			: centroid(b),
			split_bucket(split),
			bucket_count(num),
			axis(axis_)
		{ }
		bool operator()(const UMPrimitivePtr p) const;

		int split_bucket;
		int bucket_count;
		int axis;
		const umbase::UMBox &centroid;
	};

	bool compare_bucket::operator()(const UMPrimitivePtr p) const {
		int b = static_cast<int>(bucket_count * (
			(p->box().center()[axis] - centroid.minimum()[axis]) /
			(centroid.maximum()[axis] - centroid.minimum()[axis])
			));

		if (b == bucket_count)  { b = bucket_count-1; }
		assert(b >= 0 && b < bucket_count);
		return b <= split_bucket;
	}

	struct compare_point {
		compare_point(int axis_) : axis(axis_) {}
		bool operator()(const UMPrimitivePtr a, const UMPrimitivePtr b) const {
			return a->box().center()[axis] < b->box().center()[axis];
		}
		int axis;
	};

	int maximum_axis(const umbase::UMBox& box) { 
		umbase::UMVec3d v =box.maximum() - box.minimum();
		if (v.x > v.y && v.x > v.z) {
			return 0;
		} else if (v.y > v.z){
			return 1;
		} else {
			return 2;
		}
	}

	/**
	 *
	 * @param [out] ordered_primitives
	 * @param [in] scene
	 * @param [in] start
	 * @param [in] end
	 */
	UMBvhNodePtr build_middle_split(
		unsigned int& total_node_count,
		UMPrimitiveList& ordered_primitives, 
		UMPrimitiveList& primitives, 
		int start, 
		int end,
		int& depth)
	{
		const int count = end - start;
		
		++total_node_count;

		umbase::UMBox box_centroid;
		for (int i = start; i < end; ++i)
		{
			UMPrimitivePtr primitive = primitives.at(i);
			box_centroid.extend(primitive->box().center());
		}

		const int axis = maximum_axis(box_centroid);
		
		UMBvhNodePtr node(std::make_shared<UMBvhNode>());

		// create leaf
		if (depth == 0 || count <= 4 || box_centroid.maximum()[axis] == box_centroid.minimum()[axis])
		{
			const int start_index = static_cast<int>(ordered_primitives.size());
			umbase::UMBox box_all;
			for (int i = start; i < end; ++i)
			{
				UMPrimitivePtr primitive = primitives.at(i);
				box_all.extend(primitive->box());
				ordered_primitives.push_back(primitive);
			}
			node->init_as_leaf(box_all, start_index, start_index + count);
			return node;
		}

		// create branch
		umbase::UMVec3d centroid = box_centroid.center();
		UMPrimitiveList::iterator middle = std::partition(
			primitives.begin() + start, 
			primitives.begin() + end, 
			before_middle(axis, centroid[axis]));
		int middle_index = static_cast<int>(std::distance(primitives.begin(), middle));
		if (middle_index == start || middle_index == end) {
			// split equal counts
			middle_index = (start + end) / 2;
			std::nth_element(
				primitives.begin() + start,
				primitives.begin() + middle_index,
				primitives.begin() + end,
				before_less(axis));
		}

		int left_depth = depth - 1;
		int right_depth = depth - 1;
		node->init_as_branch(
			build_middle_split(total_node_count, ordered_primitives, primitives, start, middle_index, left_depth),
			build_middle_split(total_node_count, ordered_primitives, primitives, middle_index, end, right_depth),
			axis);

		if (left_depth < right_depth)
		{
			depth = left_depth;
		}
		else
		{
			depth = right_depth;
		}

		return node;
	}
	
	/**
	 *
	 * @param [out] ordered_primitives
	 * @param [in] scene
	 * @param [in] start
	 * @param [in] end
	 */
	UMBvhNodePtr build_sah(
		unsigned int& total_node_count,
		UMPrimitiveList& ordered_primitives, 
		UMPrimitiveList& primitives, 
		int start, 
		int end,
		int& depth)
	{
		const int count = end - start;
		
		++total_node_count;
		
		umbase::UMBox box_all;
		umbase::UMBox box_centroid;
		for (int i = start; i < end; ++i)
		{
			UMPrimitivePtr primitive = primitives.at(i);
			box_all.extend(primitive->box());
			box_centroid.extend(primitive->box().center());
		}
		const int axis = maximum_axis(box_centroid);
		
		UMBvhNodePtr node(std::make_shared<UMBvhNode>());
		
		// create leaf
		if (depth == 0 || box_centroid.maximum()[axis] == box_centroid.minimum()[axis])
		{
			const int start_index = static_cast<int>(ordered_primitives.size());
			for (int i = start; i < end; ++i)
			{
				UMPrimitivePtr primitive = primitives.at(i);
				ordered_primitives.push_back(primitive);
			}
			node->init_as_leaf(box_all, start_index, start_index + count);
			return node;
		}
		
		// Partition primitives into two sets and build children
		int middle_index = (start + end) / 2;
		{
			// Allocate _BucketInfo_ for SAH partition buckets
			const int buckets_count = 12;
			struct BucketInfo {
				BucketInfo() { count = 0; }
				int count;
				umbase::UMBox bounds;
			};
			BucketInfo buckets[buckets_count];
			
			// Initialize _BucketInfo_ for SAH partition buckets
			for (int i = start; i < end; ++i)
			{
				int b = static_cast<int>(buckets_count * 
					((primitives.at(i)->box().center()[axis] - box_centroid.minimum()[axis]) /
					(box_centroid.maximum()[axis] - box_centroid.minimum()[axis])));
				if (b == buckets_count) { 
					b = buckets_count-1;
				}
				assert(b >= 0 && b < buckets_count);
				buckets[b].count++;
				buckets[b].bounds.extend(primitives[i]->box());
			}
			
			// Compute costs for splitting after each bucket
			double cost[buckets_count-1];
			for (int i = 0; i < buckets_count-1; ++i) {
				umbase::UMBox b0;
				umbase::UMBox b1;
				int count0 = 0;
				int count1 = 0;
				for (int k = 0; k <= i; ++k) {
					b0.extend(buckets[k].bounds);
					count0 += buckets[k].count;
				}
				for (int k = i+1; k < buckets_count; ++k) {
					b1.extend(buckets[k].bounds);
					count1 += buckets[k].count;
				}
				cost[i] = sah(
					box_all.area(),
					b0.area(), count0,
					b1.area(), count1);
			}
			
			// Find bucket to split at that minimizes SAH metric
			double min_cost = cost[0];
			unsigned int min_cost_split = 0;
			for (int i = 1; i < buckets_count-1; ++i) {
				if (cost[i] < min_cost) {
					min_cost = cost[i];
					min_cost_split = i;
				}
			}
			
			// Either create leaf or split primitives at selected SAH bucket
			if (count > 255 || min_cost < count) {
				UMPrimitiveList::iterator middle = std::partition(
					primitives.begin() + start, 
					primitives.begin() + end, 
					compare_bucket(min_cost_split, buckets_count, axis, box_centroid));
				middle_index = static_cast<int>(std::distance(primitives.begin(), middle));
				if (middle_index <= start || middle_index >= end)
				{
					middle_index = (start + end) / 2;
				}
			}
			else
			{
				// create leaf
				int start_index = static_cast<int>(ordered_primitives.size());
				for (int i = start; i < end; ++i) {
					UMPrimitivePtr primitive = primitives.at(i);
					ordered_primitives.push_back(primitive);
				}
				node->init_as_leaf(box_all, start_index, start_index + count);
				return node;
			}
		}

		--depth;
		node->init_as_branch(
			build_sah(total_node_count, ordered_primitives, primitives, start, middle_index, depth),
			build_sah(total_node_count, ordered_primitives, primitives, middle_index, end, depth),
			axis);

		return node;
	}

	/**
	 * @param [out] dst_node_list destination node list
	 * @param [in] root recursive root
	 * @param [in] offset current index
	 */
	void flatten(UMBvhNodeList& dst_node_list, UMBvhNodePtr root, unsigned int& offset)
	{
		if (!root) return;
		root->flat_index_ = offset;
		dst_node_list.at(root->flat_index_) = root;
		++offset;

		flatten(dst_node_list, root->left_, offset);
		flatten(dst_node_list, root->right_, offset);
	}

} // anonymouse namespace

namespace umrt
{

static bool intersect_box(
	const umbase::UMBox& bounds, 
	const UMRay& ray,
	const umbase::UMVec3d& inv_dir, 
	const umbase::UMVec3i& dir_is_negative,
	double closest_distance)
{
	double interval_min = ray.tmin();
	double interval_max = closest_distance;

	// intersection of x and y slabs
	double txmin =  (bounds[  dir_is_negative[0]].x - ray.origin().x) * inv_dir.x;
	double txmax =  (bounds[1-dir_is_negative[0]].x - ray.origin().x) * inv_dir.x;
	if (txmin > interval_min) interval_min = txmin;
	if (txmax < interval_max) interval_max = txmax;
	if (interval_min > interval_max) return false;
	
	double tymin = (bounds[  dir_is_negative[1]].y - ray.origin().y) * inv_dir.y;
	double tymax = (bounds[1-dir_is_negative[1]].y - ray.origin().y) * inv_dir.y;
	if (tymin > interval_min) interval_min = tymin;
	if (tymax < interval_max) interval_max = tymax;
	if (interval_min > interval_max) return false;

	// intersection against z slab
	double tzmin = (bounds[  dir_is_negative[2]].z - ray.origin().z) * inv_dir.z;
	double tzmax = (bounds[1-dir_is_negative[2]].z - ray.origin().z) * inv_dir.z;;
	if (tzmin > interval_min) interval_min = tzmin;
	if (tzmax < interval_max) interval_max = tzmax;
	return (interval_min <= interval_max) ;
}

/**
 * build bvh from primitive list
 */
bool UMBvh::build(UMPrimitiveList& primitives)
{
	ordered_primitives_.clear();
	node_list_.clear();

	const int primitive_count = static_cast<int>(primitives.size());
	if (primitive_count <= 0) return false;
	
	ordered_primitives_.reserve(primitive_count);
	unsigned int total_node_count = 0;

	int max_depth = (std::numeric_limits<int>::max)();

	// create bvh node tree
	UMBvhNodePtr root = build_middle_split(
	//UMBvhNodePtr root = build_sah(
		total_node_count,
		ordered_primitives_, 
		primitives, 
		0, 
		primitive_count,
		max_depth);

	if (!root) return false;
	if (total_node_count == 0) return false;

	int depth = (std::numeric_limits<int>::max)() - max_depth;
	printf("nodes : %d\n", total_node_count);
	printf("max depth : %d\n", depth);

	// flatten to list
	node_list_.resize(total_node_count);
	unsigned int offset = 0;
	flatten(node_list_, root, offset);

	return true;
}

/**
 * (for debug) get box list
 */
umbase::UMBoxList UMBvh::create_box_list() const
{
	umbase::UMBoxList box_list;
	const int box_count = static_cast<int>(node_list_.size());
	box_list.resize(box_count);
	for (int i = 0; i < box_count; ++i)
	{
		const umbase::UMBox& box = node_list_.at(i)->box_;
		umbase::UMBoxPtr newbox(new umbase::UMBox(box.minimum(), box.maximum()));
		box_list.at(i) = newbox;
	}
	return box_list;
}

/**
 * ray intersection
 */
bool UMBvh::intersects(const UMRay& ray, UMShaderParameter& param) const
{
	if (node_list_.empty()) return false;
	
	umbase::UMVec3d inv_dir(1.0 / ray.direction().x, 1.0 / ray.direction().y, 1.0 / ray.direction().z);
	umbase::UMVec3i dir_is_negative(inv_dir.x < 0, inv_dir.y < 0, inv_dir.z < 0);
	
	double closest_distance = (std::numeric_limits<double>::max)();
	UMShaderParameter parameter;
	
	int box_intersect_count = 0;
	int triangle_intersect_count = 0;

	bool hit = false;
	unsigned int branch_stack[1024];
	unsigned int branch_stack_index = 0;

	unsigned int primitive_count = static_cast<unsigned int>(ordered_primitives_.size());

	int count = 0;
	for (unsigned int i = 0; ; )
	{
		const UMBvhNodePtr& node = node_list_[i];
		//if (node->box_.intersects(ray))
		if (intersect_box(node->box_, ray, inv_dir, dir_is_negative, closest_distance))
		{
			++box_intersect_count;
			if (node->is_leaf())
			{
				for (int k = node->start_index_; k < node->end_index_; ++k)
				{
					if (ordered_primitives_[k]->intersects(ray, parameter))
					{
						++triangle_intersect_count;
						if (parameter.distance < closest_distance)
						{
							++count;
							closest_distance = parameter.distance;
							param = parameter;
							hit = true;
						}
					}
				}
				// not hit. branch stack is empty.
				if (branch_stack_index == 0) break;
				// not hit. branch stack is exist. pop.
				i = branch_stack[--branch_stack_index];
			}
			// is branch
			else
			{
				if (dir_is_negative[node->axis_])
				{
					// push left branch index
					branch_stack[branch_stack_index++] = i + 1;
					// go to right
					i = node->right_->flat_index_;
				}
				else
				{
					// push right branch index
					branch_stack[branch_stack_index++] = node->right_->flat_index_;
					// go to left
					++i;
				}
			}
		}
		else
		{
			// not hit. branch stack is empty.
			if (branch_stack_index == 0) break;
			// not hit. branch stack is exist. pop.
			i = branch_stack[--branch_stack_index];
		}
	}
	return hit;
}

/**
 * ray intersection
 */
bool UMBvh::intersects(const UMRay& ray) const
{
	if (node_list_.empty()) return false;
	
	umbase::UMVec3d inv_dir(1.0 / ray.direction().x, 1.0 / ray.direction().y, 1.0 / ray.direction().z);
	umbase::UMVec3i dir_is_negative(inv_dir.x < 0, inv_dir.y < 0, inv_dir.z < 0);
	
	bool hit = false;
	unsigned int branch_stack[1024];
	unsigned int branch_stack_index = 0;
	for (unsigned int i = 0; ; )
	{
		const UMBvhNodePtr& node = node_list_[i];
		//if (node->box_.intersects(ray))
		if (intersect_box(node->box_, ray, inv_dir, dir_is_negative, ray.tmax()))
		{
			if (node->is_leaf())
			{
				for (int k = node->start_index_; k < node->end_index_; ++k)
				{
					if (ordered_primitives_[k]->intersects(ray))
					{
						return true;
					}
				}
				// not hit. branch stack is empty.
				if (branch_stack_index == 0) break;
				// not hit. branch stack is exist. pop.
				i = branch_stack[--branch_stack_index];
			}
			// is branch
			else
			{
				if (dir_is_negative[node->axis_])
				{
					// push left branch index
					branch_stack[branch_stack_index++] = i + 1;
					// go to right
					i = node->right_->flat_index_;
				}
				else
				{
					// push right branch index
					branch_stack[branch_stack_index++] = node->right_->flat_index_;
					// go to left
					++i;
				}
			}
		}
		else
		{
			// not hit. branch stack is empty.
			if (branch_stack_index == 0) break;
			// not hit. branch stack is exist. pop.
			i = branch_stack[--branch_stack_index];
		}
	}
	return false;
}

/**
 * get box
 */
const umbase::UMBox& UMBvh::box() const
{
	if (!node_list_.empty())
	{
		return (*node_list_.begin())->box_;
	}
	static umbase::UMBox empty;
	return empty;
}

} // umrt
