#include <Processing.NDI.Lib.h>
#include "Unity/IUnityInterface.h"

#include <thread>

//
// Global
//

extern "C" bool UNITY_INTERFACE_EXPORT NDI_Initialize()
{
	return NDIlib_initialize();
}

extern "C" void UNITY_INTERFACE_EXPORT NDI_Finalize()
{
	NDIlib_destroy();
}

//
// Sender
//

extern "C" void UNITY_INTERFACE_EXPORT *NDI_CreateSender(const char* name)
{
	NDIlib_send_create_t desc;
	desc.p_ndi_name = name;
	return NDIlib_send_create(&desc);
}

extern "C" void UNITY_INTERFACE_EXPORT NDI_DestroySender(void* sender)
{
	NDIlib_send_destroy(sender);
}

extern "C" void UNITY_INTERFACE_EXPORT NDI_SendFrame(void* sender, void* data, int width, int height)
{
	NDIlib_video_frame_v2_t frame;
	
	frame.xres = width;
	frame.yres = height;
	frame.FourCC = NDIlib_FourCC_type_BGRX;
	frame.frame_format_type = NDIlib_frame_format_type_interleaved;
	frame.frame_rate_N = 60;
	frame.frame_rate_D = 1;
	frame.p_data = static_cast<uint8_t*>(data);
	frame.line_stride_in_bytes = width * 4;

	NDIlib_send_send_video_v2(sender, &frame);
}

//
// Receiver
//

namespace
{
	NDIlib_video_frame_v2_t receiver_frame;
}

extern "C" int UNITY_INTERFACE_EXPORT NDI_GetFrameWidth(void* receiver)
{
	return receiver_frame.xres;
}

extern "C" int UNITY_INTERFACE_EXPORT NDI_GetFrameHeight(void* receiver)
{
	return receiver_frame.yres;
}

extern "C" void UNITY_INTERFACE_EXPORT *NDI_GetFrameData(void* receiver)
{
	return receiver_frame.p_data;
}

extern "C" void UNITY_INTERFACE_EXPORT *NDI_CreateReceiver()
{
	auto find = NDIlib_find_create_v2(&NDIlib_find_create_t());
	if (find == nullptr) return nullptr;

	const NDIlib_source_t* sources = nullptr;

	uint32_t count;

	for (auto i = 0; i < 5; i++)
	{
		NDIlib_find_wait_for_sources(find, 1000);

		sources = NDIlib_find_get_current_sources(find, &count);
		if (count > 0) break;
	}
	if (count == 0)
	{
		NDIlib_find_destroy(find);
		return nullptr;
	}
	/*
	NDIlib_find_wait_for_sources(find, 1000);

	const NDIlib_source_t* sources = nullptr;
	uint32_t count;
	sources = NDIlib_find_get_current_sources(find, &count);
	
	if (count == 0)
	{
		NDIlib_find_destroy(find);
		return nullptr;
	}
	*/

	auto recv = NDIlib_recv_create_v2(&NDIlib_recv_create_t(sources[0]));

	NDIlib_find_destroy(find);

	return recv;
}

extern "C" void UNITY_INTERFACE_EXPORT NDI_DestroyReceiver(void* receiver)
{
	NDIlib_recv_destroy(receiver);
}

extern "C" bool UNITY_INTERFACE_EXPORT NDI_ReceiveFrame(void* receiver)
{
	auto type = NDIlib_recv_capture_v2(receiver, &receiver_frame, nullptr, nullptr, 1000);
	return type == NDIlib_frame_type_video;
}

extern "C" void UNITY_INTERFACE_EXPORT NDI_FreeFrame(void* receiver)
{
	NDIlib_recv_free_video_v2(receiver, &receiver_frame);
}