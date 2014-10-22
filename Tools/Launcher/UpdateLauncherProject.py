
"""
Platform Specifies
"""
import platform
osbits, pos = platform.architecture()
osplat = platform.system()

print osbits, osplat

import os,re

class SourceGroup:
	def __init__(self):
		self.headers = []
		self.sources = []

pat_header = re.compile(r'[a-zA-Z|\d|\\|/|_|.]+\.(h|hpp)$')
pat_sources = re.compile(r'[a-zA-Z|\d|\\|/|_|.]+\.(cpp|cc|c)$')

def findSourceFiles(arg,dirname,names):
	for filespath in names:
		pathstr = os.path.join(dirname,filespath)
		pathstr = pathstr.replace('\\', '/')
		if os.path.isfile(pathstr):
			if pat_header.match(pathstr):
				arg.headers.append(pathstr)
			if pat_sources.match(pathstr):
				arg.sources.append(pathstr)

				
def update_launcher_qt_pro():
	dep_file = open('launcher.pro', 'w')
	dep_file.write('TARGET = launcher\n')
	dep_file.write('CONFIG -= QT\n')
	
	dep_file.write('include (d:/lib/physx_x64.pri)\n')
	dep_file.write('include (d:/lib/openal_x64.pri)\n')
	
	includepath = 'INCLUDEPATH+='
	inc_path = {'$$PWD/../../Include', '$$PWD/../../Source', '$$PWD/../../Source/Renderer'}
	for ipath in inc_path:
		includepath += '\\\n\t' + ipath
	
	dep_file.write(includepath + '\n')
	
	path_core		=	'../../Source/Core'
	path_engine 	= 	'../../Source/Engine'
	path_renderer	=	'../../Source/Renderer'
	path_render_gl	=	'../../Source/Renderer/OpenGL'
	path_render_post=	'../../Source/Renderer/Postprocess'
	path_phys		= 	'../../Source/Physics'
	path_entity 	= 	'../../Source/Entity'
	path_audio		= 	'../../Source/Audio'
	
	source_paths 	= { path_core, path_engine, path_renderer, path_render_gl, path_phys, path_entity, path_audio }
	
	src_grp = SourceGroup()

	for path_module in source_paths:			
		os.path.walk(path_module, findSourceFiles, src_grp)
	
	path_platform_launcher = ''

	if osplat=='Windows':
		path_platform_launcher = '../../Source/Platform/Windows'
	elif osplat=='Linux':
		path_platform_launcher = '../../Source/Platform/Linux'
	else:
		dep_file.write('include (d:/lib/glfw_x64.pri)\n')
		dep_file.write('DEFINES += USE_GLFW\n')
		print 'GLFW Specified!'
		path_platform_launcher = '../../Source/Platform/GLFW'

	os.path.walk(path_platform_launcher, findSourceFiles, src_grp)

	headers = 'HEADERS+='
	sources = 'SOURCES+='
	
	pch = 'PRECOMPILED_HEADER +=$$PWD/../../Include/Config/Prerequisities.h\n'
	
	for header in src_grp.headers:
		headers += '\\\n\t' + header
	for source in src_grp.sources:
		sources += '\\\n\t' + source

	dep_file.write(pch)
	dep_file.write(headers+'\n')
	dep_file.write(sources+'\n')
	
	dep_file.write('CONFIG += c++11\n')
	dep_file.write('DEFINES += ENABLE_SSE\n')
	dep_file.write('win32: RC_FILE = ../../Project/kaleido3d.rc\n')

	dep_file.close()


if __name__=="__main__":

	osplat = 'glfw'
	
	update_launcher_qt_pro()
	print 'Launcher.pro generated !'
	os.system("if exists build rmdir build")
	os.system("(if not exists build mkdir build) && cd build && qmake -tp vc ..\launcher.pro")
	print 'MSVC proj generated !'
	os.system("pause")
