/**
 * @file UMWSIO.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_WSIO
#include "UMWSIO.h"
#include "UMWSIOEventType.h"
#include "UMScene.h"
#include "UMEvent.h"
#include "UMCamera.h"
#include <thread>
#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
typedef websocketpp::lib::lock_guard<websocketpp::lib::mutex> scoped_lock;

namespace umdraw
{
	class UMScene;
	typedef std::shared_ptr<UMScene> UMScenePtr;
} // umdraw

namespace umwsio
{

class UMWSIO::Impl
{
public:
	typedef websocketpp::server<websocketpp::config::asio> server;

	Impl()
		: model_loaded_event_(std::make_shared<umbase::UMEvent>(eWSIOEventModelLoaded))
		, is_loaded_(false)
	{}
	~Impl() 
	{
		if (thread_.joinable())
		{
			ioserver_.get_io_service().stop();
			thread_.join();
		}
	}

	void start_server(umdraw::UMScenePtr scene)
	{
		scene_ = scene;
		thread_ = std::thread([this] { do_(); });
	}
	
	void done()
	{
		is_loaded_ = false;
	}

	void on_message(server* s, websocketpp::connection_hdl hdl, server::message_ptr msg) 
	{
		//scoped_lock guard(lock_);
		const std::string& payload = msg->get_payload();
		//std::cout << payload << std::endl;
		if (payload == "send_data_start")
		{
			scene_->mutable_mesh_group_list().clear();
			s->send(hdl, "send_data_start_done", websocketpp::frame::opcode::text);
		}
		else if (payload == "send_data_end")
		{
			is_loaded_ = true;
		}
		else
		{
			const size_t camera_count = scene_->mutable_camera_list().size();
			if (scene_->load_from_memory(payload))
			{
				const size_t imported_camera_count = scene_->mutable_camera_list().size() - camera_count;
				if (imported_camera_count > 0)
				{
					umdraw::UMCameraPtr camera = scene_->mutable_camera_list().at(camera_count);
					scene_->camera()->mutable_global_transform() = camera->global_transform();
					scene_->camera()->mutable_local_transform() = camera->local_transform();

					//const umbase::UMMat44d& mat = scene_->camera()->global_transform();
					//std::cout << "camera" << std::endl;
					//std::cout << 
					//	mat.m[0][0] << " " <<  mat.m[0][1] << " " << mat.m[0][2] << " " << mat.m[0][3] << " " << std::endl;
					//std::cout << 
					//	mat.m[1][0] << " " <<  mat.m[1][1] << " " << mat.m[1][2] << " " << mat.m[1][3] << " " << std::endl;
					//std::cout << 
					//	mat.m[2][0] << " " <<  mat.m[2][1] << " " << mat.m[2][2] << " " << mat.m[2][3] << " " << std::endl;
					//std::cout << 
					//	mat.m[3][0] << " " <<  mat.m[3][1] << " " << mat.m[3][2] << " " << mat.m[3][3] << " " << std::endl;

					scene_->camera()->update_from_node();
				}
			}
			else
			{
				scene_->init(scene_->width(), scene_->height());
			}
			s->send(hdl, "send_data_done", websocketpp::frame::opcode::text);
		}
	}

	bool is_loaded() const
	{
		return is_loaded_;
	}
	
	umbase::UMEventPtr model_loaded_event()
	{
		return model_loaded_event_;
	}

private:
	void do_()
	{
		ioserver_.set_message_handler(bind(&UMWSIO::Impl::on_message, this, &ioserver_, _1, _2));
		ioserver_.init_asio();
		ioserver_.listen(9002);
		ioserver_.start_accept();
		ioserver_.run();
	}
	websocketpp::lib::mutex lock_;
	server ioserver_;
	umdraw::UMScenePtr scene_;
	std::thread thread_;
	umbase::UMEventPtr model_loaded_event_;
	bool is_loaded_;
};

/**
 * constructor
 */
UMWSIO::UMWSIO()
	: impl_(new UMWSIO::Impl())
{
}

/**
 * destructor
 */
UMWSIO::~UMWSIO()
{
}

/**
 * init
 */
bool UMWSIO::init()
{
	return false;
}

/**
 * add umdraw scene
 */
bool UMWSIO::start_server(umdraw::UMScenePtr scene)
{
	impl_->start_server(scene);
	return true;
}

/**
 * get model loaded event
 */
umbase::UMEventPtr UMWSIO::model_loaded_event()
{
	return impl_->model_loaded_event();
}

bool UMWSIO::is_loaded() const
{
	return impl_->is_loaded();
}

void UMWSIO::done()
{
	impl_->done();
}

} // umwsio

#endif // WITH_WSIO
