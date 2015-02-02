/*
 * libcorkscrew_stab.cpp
 *
 *  Created on: Dec 20, 2014
 *      Author: l_sinyakov
 */
#include "libcorkscrew_stab.h"
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>

#include "Base/BaseTypes.h"
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"
#include "FileSystem/File.h"

t_unwind_backtrace_signal_arch unwind_backtrace_signal_arch = NULL;
t_acquire_my_map_info_list acquire_my_map_info_list = NULL;
t_release_my_map_info_list release_my_map_info_list = NULL;
t_get_backtrace_symbols get_backtrace_symbols = NULL;
t_free_backtrace_symbols free_backtrace_symbols = NULL;

bool DynLoadLibcorkscrew()
{
	LOGE("FRAME_STACK loading corkscrew");
	if (unwind_backtrace_signal_arch != NULL)
		return true;

	void* libcorkscrew = dlopen("/system/lib/libcorkscrew.so", RTLD_NOW);
	if (libcorkscrew)
	{
		unwind_backtrace_signal_arch = (t_unwind_backtrace_signal_arch) dlsym(
				libcorkscrew, "unwind_backtrace_signal_arch");
		if (!unwind_backtrace_signal_arch)
		{
			LOGE("FRAME_STACK unwind_backtrace_signal_arch not found");
			return false;
		}
		

		acquire_my_map_info_list = (t_acquire_my_map_info_list) dlsym(
				libcorkscrew, "acquire_my_map_info_list");
		if (!acquire_my_map_info_list)
		{
			LOGE("FRAME_STACK acquire_my_map_info_list not found");
			return false;
		}
		
		get_backtrace_symbols = (t_get_backtrace_symbols) dlsym(libcorkscrew,
				"get_backtrace_symbols");
		if (!get_backtrace_symbols)
		{
			LOGE("FRAME_STACK get_backtrace_symbols not found");
			return false;
		}

		free_backtrace_symbols = (t_free_backtrace_symbols) dlsym(libcorkscrew,
				"free_backtrace_symbols");
		if (!free_backtrace_symbols)
		{
			LOGE("FRAME_STACK free_backtrace_symbols not found");
			return false;
		}

		release_my_map_info_list = (t_release_my_map_info_list) dlsym(
				libcorkscrew, "release_my_map_info_list");
		if (!release_my_map_info_list)
		{
			LOGE("FRAME_STACK release_my_map_info_list not found");
			return false;
		}
		return true;
	}
	else
	{
		LOGE("FRAME_STACK libcorkscrew not found");

	}
	return false;
}

