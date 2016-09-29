import os
import build_utils

def get_supported_targets_for_build_platform(platform):
	if platform == 'win32':
		return ['win32', 'win10', 'android']
	else:
		return ['macos', 'ios', 'android']

def get_dependencies_for_target(target):
	return []

def get_supported_build_platforms():
	return ['win32', 'darwin']

def build_for_target(target, working_directory_path, root_project_path):
	if target == 'win32':
		return _build_win32(working_directory_path, root_project_path)
	elif target == 'win10':
		return _build_win10(working_directory_path, root_project_path)
	elif target == 'macos':
		return _build_macos(working_directory_path, root_project_path)
	elif target == 'ios':
		return _build_ios(working_directory_path, root_project_path)
	elif target == 'android':
		return _build_android(working_directory_path, root_project_path)

def get_download_url():
	return 'http://pyyaml.org/download/libyaml/yaml-0.1.7.tar.gz'

def _download_and_extract(working_directory_path):
	source_folder_path = os.path.join(working_directory_path, 'libyaml_source')
	url = get_download_url()
	build_utils.download_and_extract(url, working_directory_path, source_folder_path, build_utils.get_url_file_name_no_ext(url))	
	return source_folder_path

def _patch_sources(source_folder_path, working_directory_path):
	try:
		if _patch_sources.did:
			return
	except AttributeError:
		pass

	build_utils.apply_patch(os.path.abspath('patch.diff'), working_directory_path)

	_patch_sources.did = True

def _build_win32(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)
	_patch_sources(source_folder_path, working_directory_path)

	build_utils.build_and_copy_libraries_win32_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'yaml.sln', 'yaml',
		'yaml.lib', 'yaml.lib',
		'libyaml_wind.lib', 'libyaml_win.lib',
		'libyaml_wind.lib', 'libyaml_win.lib')

	_copy_headers(source_folder_path, root_project_path)

	return True

def _build_win10(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)
	_patch_sources(source_folder_path, working_directory_path)

	build_utils.build_and_copy_libraries_win10_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'yaml.sln', 'yaml',
		'yaml.lib', 'yaml.lib',
		'libyaml_wind.lib', 'libyaml_win.lib',
		'libyaml_wind.lib', 'libyaml_win.lib',
		'libyaml_wind.lib', 'libyaml_win.lib',
		['-DWIN10=1'])

	_copy_headers(source_folder_path, root_project_path)

	return True

def _build_macos(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)
	_patch_sources(source_folder_path, working_directory_path)

	build_utils.build_and_copy_libraries_macos_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'yaml.xcodeproj', 'yaml',
		'libyaml.a',
		'libyaml_macos.a')

	_copy_headers(source_folder_path, root_project_path)

	return True

def _build_ios(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)
	_patch_sources(source_folder_path, working_directory_path)

	build_utils.build_and_copy_libraries_ios_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'yaml.xcodeproj', 'yaml',
		'libyaml.a',
		'libyaml_ios.a')

	_copy_headers(source_folder_path, root_project_path)

	return True

def _build_android(working_directory_path, root_project_path):
	source_folder_path = _download_and_extract(working_directory_path)
	_patch_sources(source_folder_path, working_directory_path)

	build_utils.build_and_copy_libraries_android_cmake(
		os.path.join(working_directory_path, 'gen'), source_folder_path, root_project_path,
		'libyaml.a',
		'libyaml.a')

	_copy_headers(source_folder_path, root_project_path)

	return True

def _copy_headers(source_folder_path, root_project_path):
	include_path = os.path.join(root_project_path, 'Libs/include/yaml')
	build_utils.copy_files(source_folder_path, include_path, 'include/*.h')