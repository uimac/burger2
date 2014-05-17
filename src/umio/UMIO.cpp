/**
 *
 * @file UMIO.cpp
 *
 * @author tori31001 at gmail.com
 *
 *
 * Copyright (C) 2012 Kazuma Hatta
 * Dual licensed under the MIT or GPL Version 2 licenses. 
 *
 */
#include "UMIO.h"
#include "UMObject.h"

#if defined(WITH_FBX2014) || defined(WITH_FBX2013) || defined(WITH_FBX2011)
#define WITH_FBX
#endif

#ifdef WITH_FBX
#include "UMFbx.h"
#endif

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>

#ifdef WITH_BOOST_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif

namespace umio
{
	
static bool isutf8(const std::string &str)
{
	for (size_t i = 0; i < str.size(); ++i)
	{
		const char c = str.at(i);
		if ((c & 0x80) != 0x00) 
		{
			if((c & 0xe0) != 0xc0 && (c & 0xc0) != 0x80)
			{
				if((c & 0xf0) != 0xe0 && (c & 0xc0) != 0x80 && (c & 0xc0) != 0x80 )
				{
					return false;
				}
			}
		}
	}
	return true;
}

/**
 * load file
 */
UMObjectPtr UMIO::load(std::string path, const UMIOSetting& setting)
{
	bool is_fbx_load = false;
	UMIOSetting::UMImpSettingMap::const_iterator bt = setting.bl_imp_prop_map().begin();
	for (; bt != setting.bl_imp_prop_map().end(); ++bt)
	{
		UMIOSetting::EUMImpSettingType type = bt->first;
		bool val = bt->second;

		if (type == UMIOSetting::eUMImpFBX)
		{
			is_fbx_load = val;
		}
	}
	
#ifdef WITH_FBX
	// load by fbx sdk
	if (is_fbx_load)
	{
		UMFbx fbx;
		if (UMObjectPtr obj = fbx.load(path, setting))
		{
			return obj;
		}
	}
#endif
	
#ifdef WITH_BOOST_SERIALIZATION
	// load bos
	try {

		if (isutf8(path))
		{
			std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> convert;
			std::u16string u16str = convert.from_bytes(path);

			// convert bos to fbx or other format 
			std::ifstream file(u16str.c_str(), std::ios::in | std::ios::binary);
			boost::archive::binary_iarchive ia(file);
		
			umio::UMObjectPtr obj = umio::UMObject::create_object();
		
			ia >> (*obj);
		
			file.close();
			if (UMObject::re_bind_all_nodes(obj))
			{
				return obj;
			}
		}
		else
		{
			// convert bos to fbx or other format 
			std::ifstream file(path, std::ios::in | std::ios::binary);
			boost::archive::binary_iarchive ia(file);
		
			umio::UMObjectPtr obj = umio::UMObject::create_object();
		
			ia >> (*obj);
		
			file.close();
			if (UMObject::re_bind_all_nodes(obj))
			{
				return obj;
			}
		}
	}
	catch (...) {
		//std::cout << "unknown excaption" << std::endl;
		return UMObjectPtr();
	}
#endif // WITH_BOOST_SERIALIZATION

	return UMObjectPtr();
}

/**
 * save file
 */
bool UMIO::save(std::string path, UMObjectPtr object, const UMIOSetting& setting)
{
	bool is_fbx_save = false;
	UMIOSetting::UMExpSettingMap::const_iterator bt = setting.bl_exp_prop_map().begin();
	for (; bt != setting.bl_exp_prop_map().end(); ++bt)
	{
		UMIOSetting::EUMExpSettingType type = bt->first;
		bool val = bt->second;

		if (type == UMIOSetting::eUMExpFBX)
		{
			is_fbx_save = val;
		}
	}
	
#ifdef WITH_FBX
	// save by fbx sdk
	if (is_fbx_save)
	{
		UMFbx fbx;
		return fbx.save(path, object, setting);
	}
#endif
	
#ifdef WITH_BOOST_SERIALIZATION
	// save bos
	try {
		// convert fbx or other format to bos
		std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
		boost::archive::binary_oarchive oa(file);

		oa << (*object);

		file.flush();
		file.close();

		return true;
	}
	catch (...) {
		//std::cout << "unknown excaption" << std::endl;
		return false;
	}
#endif // WITH_BOOST_SERIALIZATION

	return false;
}

/**
 * load file
 */
UMObjectPtr UMIO::load_bos_from_memory(const std::string& src, const UMIOSetting& setting)
{
	bool is_fbx_load = false;
	UMIOSetting::UMImpSettingMap::const_iterator bt = setting.bl_imp_prop_map().begin();
	for (; bt != setting.bl_imp_prop_map().end(); ++bt)
	{
		UMIOSetting::EUMImpSettingType type = bt->first;
		bool val = bt->second;

		if (type == UMIOSetting::eUMImpFBX)
		{
			is_fbx_load = val;
		}
	}
	
#ifdef WITH_BOOST_SERIALIZATION
	// load bos
	try {
		std::istringstream stream(src);

		// convert bos to fbx or other format 
		boost::archive::binary_iarchive ia(stream);
		
		umio::UMObjectPtr obj = umio::UMObject::create_object();
		
		ia >> (*obj);
		
		if (UMObject::re_bind_all_nodes(obj))
		{
			return obj;
		}
	}
	catch (...) {
		//std::cout << "unknown excaption" << std::endl;
		return UMObjectPtr();
	}
#endif // WITH_BOOST_SERIALIZATION

	return UMObjectPtr();
}

/**
 * load settings
 */
bool UMIO::load_setting(std::string path, UMIOSetting& setting)
{
#ifdef WITH_BOOST_SERIALIZATION
	// load bos
	try {
		// convert bos to fbx or other format 
		std::ifstream file(path, std::ios::in | std::ios::binary);
		boost::archive::binary_iarchive ia(file);
		
		ia >> (setting);
		
		file.close();

		return true;
	}
	catch (...) {
		//std::cout << "unknown excaption" << std::endl;
		return UMObjectPtr();
	}
#endif // WITH_BOOST_SERIALIZATION
	return false;
}
	
/**
 * save settings
 */
bool UMIO::save_setting(std::string path, const UMIOSetting& setting)
{
#ifdef WITH_BOOST_SERIALIZATION
	// save bos
	try {
		// convert fbx or other format to bos
		std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
		boost::archive::binary_oarchive oa(file);

		oa << setting;
		
		file.flush();
		file.close();

		return true;
	}
	catch (...) {
		//std::cout << "unknown excaption" << std::endl;
		return false;
	}
#endif // WITH_BOOST_SERIALIZATION
	return false;
}

} // namespace umio
