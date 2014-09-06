/**
 * @file UMResource.cpp
 * resource
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */

#include <snappy.h>
#include <exception>
#include <assert.h>
#include "UMResource.h"
#include "UMPath.h"
#include "UMStringUtil.h"

namespace 
{
	bool compress(const umstring& file_name, std::istream &in, std::ostream &out) 
	{
		// header
		{
			out.write("U", 1);
			out.write("P", 1);
			out.write("A", 1);
			out.write("C", 1);
			unsigned int version = 1;
			out.write(reinterpret_cast<char* >(&version), 4); 
		}

		umstring name = file_name;
		unsigned int name_size = static_cast<unsigned int>(name.size() * 2);
		{
			// write name size
			out.write(reinterpret_cast<char* >(&name_size), 4); 
			// write name
			out.write(reinterpret_cast<const char* >(name.c_str()), name_size);
		}

		unsigned int total_size = static_cast<unsigned int>(in.seekg(0, std::ios::end).tellg());
		in.seekg(0, std::ios::beg);

		std::vector<char> in_block(snappy::kBlockSize);
		std::string out_buffer;
		std::vector<char> test_buffer;

		for (size_t pos = 0; pos < total_size;) 
		{
			// read
			in.read(&(*in_block.begin()), snappy::kBlockSize);
			const size_t read_size = in.gcount();
			pos += read_size;
			if (read_size > 0)
			{
				// compress
				unsigned int compressed_size = 
					static_cast<unsigned int>(snappy::Compress(&(*in_block.begin()), read_size, &out_buffer));

				// write compressed size
				out.write(reinterpret_cast<char* >(&compressed_size), 4);

				// write compressed data
				out.write(out_buffer.c_str(), compressed_size);
				if (out.bad()) { return false; }

				out_buffer.clear();
			}
			if (in.fail() && pos != total_size) 
			{
				return false;
			}
		}

		// footer
		{
			int footer = -1;
			out.write(reinterpret_cast<char* >(&footer), 4);
		}
		return true;
	}

	bool uncompress_to_memory(
		umresource::UMResource::UnpackedNameList& unpacked_name_list, 
		umresource::UMResource::UnpackedDataList& unpacked_data_list, 
		std::istream &in)
	{
		unsigned int total_size = static_cast<unsigned int>(in.seekg(0, std::ios::end).tellg());
		in.seekg(0, std::ios::beg);
		
		for (; in.tellg() < total_size;) 
		{
			// header
			unsigned int version = 0;
			{
				char U, P, A, C;
				in.read(&U, 1);
				in.read(&P, 1);
				in.read(&A, 1);
				in.read(&C, 1);
				if (! (U == 'U' && P == 'P' && A == 'A' && C == 'C'))
				{
					return false;
				}
				in.read(reinterpret_cast<char* >(&version), 4);
			}
			if (version < 1) return false;

			umstring file_name;
			unsigned int name_size = 0;
			{
				// read name size
				in.read(reinterpret_cast<char* >(&name_size), 4); 
				//printf("name size %d\n", name_size);
#ifdef WITH_EMSCRIPTEN
				// read name
				file_name.resize(name_size / 2);
				std::vector<char> buffer(name_size);
				in.read(&buffer[0], name_size);
				int name_pos = 0;
				for (int i = 0; i < name_size; i += 2, ++name_pos)
				{
					file_name[name_pos] = buffer[i];
				}
#else
				// read name
				file_name.resize(name_size / 2);
				in.read(reinterpret_cast<char* >(const_cast<char16_t*>(file_name.c_str())), name_size);
#endif
			}

			if (std::find(unpacked_name_list.begin(), unpacked_name_list.end(), file_name) != unpacked_name_list.end())
			{
				// multiple same filenames are not supported.
				return false;
			}
		
			unpacked_name_list.push_back(file_name);
			unpacked_data_list.push_back(std::string(""));
			std::string& file_buffer = unpacked_data_list.back();
			// create temporary buffer
			std::vector<char> in_block(snappy::MaxCompressedLength(snappy::kBlockSize));
			std::string block_buffer;
		
			for (;;) 
			{
				unsigned int compressed_size = 0;
				{
					// read compressed_ size
					in.read(reinterpret_cast<char* >(&compressed_size), 4);
					if (compressed_size == 0xFFFFFFFF)
					{
						// this file is end. process next file
						break;
					}
				
					// read
					in.read(&(*in_block.begin()), compressed_size);
					size_t read_size = in.gcount();
				}

				// uncompress
				if (snappy::IsValidCompressedBuffer(&(*in_block.begin()), compressed_size))
				{
					if (snappy::Uncompress(&(*in_block.begin()), compressed_size, &block_buffer))
					{
						file_buffer.reserve(file_buffer.size() + block_buffer.size());
						file_buffer.append(block_buffer);
						block_buffer.clear();
					}
				}
				
				if (in.fail()) 
				{
					return false;
				}
			}
		}
		return true;
	}
} // anonymouse namespace

namespace umresource
{

UMResource::UMResource()
{
}
/**
 * get default resource path
 */
umstring UMResource::default_resource_path()
{
	return umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("cabbage_resource.pack"));
}

/**
 * get default resource path
 */
umstring UMResource::default_model_resource_path()
{
	return umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("model_resource.pack"));
}

/**
 * pack files to dst file
 */
bool UMResource::pack(const umstring& dst_absolute_path, const FilePathList& src_absolute_path_list)
{
	try
	{
		std::ofstream out(dst_absolute_path.c_str(), std::ios::out|std::ios::binary|std::ios::trunc);

		FilePathList::const_iterator it = src_absolute_path_list.begin();
		for (; it != src_absolute_path_list.end(); ++it)
		{
			const umstring& path = *it;
			if (umbase::UMPath::exists(path))
			{
				if (umbase::UMPath::is_folder(path))
				{
					// create inner resource file
					umstring temp_path = umbase::UMPath::get_temp_absolute_path();
					std::ofstream inner_out(temp_path.c_str(), std::ios::out|std::ios::binary|std::ios::trunc);

					std::vector<umstring> dirs;
					std::vector<umstring> files;
					if (umbase::UMPath::get_child_path_list(dirs, files, path))
					{
						for (size_t i = 0; i < files.size(); ++i)
						{
							umstring& file_path = files.at(i);

							std::ifstream in(file_path.c_str(), std::ios::binary);
							if (!in) { assert(0); return false; }

							const umstring file_name = umbase::UMPath::get_file_name(file_path);
							if (!compress(file_name, in, inner_out))
							{
								assert(0);
							}
						}
					}
					
					std::ifstream in(temp_path.c_str(), std::ios::binary);
					if (!in) { assert(0); return false; }
					const umstring folder_name = umbase::UMPath::get_file_name(path);
					if (!compress(folder_name, in, out))
					{
						assert(0);
					}
					umbase::UMPath::remove_file(temp_path);
				}
				else
				{
					std::ifstream in(path.c_str(), std::ios::binary);
					if (!in) { assert(0); return false; }

					const umstring file_name = umbase::UMPath::get_file_name(path);
					if (!compress(file_name, in, out))
					{
						assert(0);
					}
				}
			}
		}
	}
	catch (...)//(const std::exception &ex) 
	{
		assert(0);
		return false;
	}
	return true;
}

/**
 * unpack files to dst directory
 */
bool UMResource::unpack(const umstring& dst_absolute_path, const umstring& src_absolute_path)
{
	return false;
}

/**
 * unpack files to memory
 */
bool UMResource::unpack_to_memory(const umstring& src_absolute_path)
{
	try
	{
		std::ifstream in(src_absolute_path.c_str(), std::ios::binary);
		uncompress_to_memory(unpacked_name_list_, unpacked_data_list_, in);
	}
	catch (...)//(const std::exception &ex) 
	{
		assert(0);
		return false;
	}
	return true;
}

/**
 * find resource data
 */
const std::string& UMResource::find_resource_data(UMResource& resource, const std::string& name)
{
	UMResource::UnpackedNameList& name_list = resource.unpacked_name_list();
	UMResource::UnpackedDataList& data_list = resource.unpacked_data_list();
	umstring utf16name = umbase::UMStringUtil::utf8_to_utf16(name);
	UMResource::UnpackedNameList::iterator it = std::find(name_list.begin(), name_list.end(), utf16name);

	if (it != name_list.end())
	{
		size_t pos = std::distance(name_list.begin(), it);
		return data_list.at(pos);
	}
	static std::string none("");
	return none;
}

} // umresource
