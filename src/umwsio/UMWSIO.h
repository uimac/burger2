/**
 * @file UMWSIO.h
 * websockets io
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
#include "UMVector.h"
#include "UMMathTypes.h"
#include "UMEvent.h"

namespace umdraw
{
	class UMScene;
	typedef std::shared_ptr<UMScene> UMScenePtr;
} // umdraw

namespace umwsio
{

class UMWSIO;
typedef std::shared_ptr<UMWSIO> UMWSIOPtr;

/**
 * websockets io
 */
class UMWSIO 
{
	DISALLOW_COPY_AND_ASSIGN(UMWSIO);
public:
	UMWSIO();
	~UMWSIO();

	/**
	 * init
	 */
	bool init();

	/**
	 * add umdraw scene
	 */
	bool start_server(umdraw::UMScenePtr scene);

	bool is_loaded() const;
	
	void done();
	/**
	 * get model loaded event
	 */
	umbase::UMEventPtr model_loaded_event();

private:
	class Impl;
	typedef std::unique_ptr<Impl> ImplPtr;
	ImplPtr impl_;
};

} // umwsio
