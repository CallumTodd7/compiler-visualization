#
# Copyright (c) 2006-2019 LOVE Development Team
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
	# Protip: run cmake like this: cmake -G "<generator>" -H. -Bbuild
	message(FATAL_ERROR "Prevented in-tree build.")
endif()

cmake_minimum_required(VERSION 3.1)

project(love)

set(LOVE_LIB_NAME liblove)

set(CMAKE_MODULE_PATH "${love_SOURCE_DIR}/extra/cmake" ${CMAKE_MODULE_PATH})
# Needed for shared libs on Linux. (-fPIC).
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

set (CMAKE_CXX_STANDARD 11)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(LOVE_X64 TRUE)
	set(LOVE_TARGET_PLATFORM x64)
else()
	set(LOVE_X86 TRUE)
	set(LOVE_TARGET_PLATFORM x86)
endif()

message(STATUS "Target platform: ${LOVE_TARGET_PLATFORM}")

if(POLICY CMP0072)
	cmake_policy(SET CMP0072 NEW)
endif()

find_package(OpenGL)

if(MEGA)
	# LOVE_MSVC_DLLS contains runtime DLLs that should be bundled with the love
	# binary (in e.g. the installer). Example: msvcp140.dll.
	set(LOVE_MSVC_DLLS ${MEGA_MSVC_DLLS})

	# LOVE_INCLUDE_DIRS contains the search directories for #include. It's mostly
	# not needed for MEGA builds, since almost all the libraries (except LuaJIT)
	# are CMake targets, causing include paths to be added automatically.
	set(LOVE_INCLUDE_DIRS)

	if(APPLE)
		# Some files do #include <SDL2/SDL.h>, but building with megasource
		# requires #include <SDL.h>.
		add_definitions(-DLOVE_MACOSX_SDL_DIRECT_INCLUDE)
	endif ()

	# SDL2 links with some DirectX libraries, and we apparently also
	# pull those libraries in for linkage because we link with SDL2.
	set(LOVE_LINK_DIRS ${SDL_LINK_DIR})

	set(LOVE_LINK_LIBRARIES
		${OPENGL_gl_LIBRARY}
		${MEGA_FREETYPE}
		${MEGA_SDL2MAIN}
		${MEGA_SDL2}
	)

	# These DLLs are moved next to the love binary in a post-build step to
	# love runnable from inside Visual Studio.
	#
	# LOVE_MOVE_DLLS can contain CMake targets, in which case the target's
	# output is assumed to be a DLL, or it can contain paths to actual files.
	# We detect whether or not each item is a target, and take the appropriate
	# action.
	set(LOVE_MOVE_DLLS
		${MEGA_SDL2}
	)
else()
	if(MSVC)
		message(FATAL_ERROR "
It is currently only possible to build with megasource on Windows.
Please see http://bitbucket.org/rude/megasource
")
	endif()

	find_package(Freetype REQUIRED)
	find_package(OpenGL REQUIRED)
	find_package(SDL2 REQUIRED)

	# required for enet
	add_definitions(-D HAS_SOCKLEN_T)

	set(LOVE_INCLUDE_DIRS
		${SDL2_INCLUDE_DIR}
		${FREETYPE_INCLUDE_DIRS}
	)

	set(LOVE_LINK_LIBRARIES
		${OPENGL_gl_LIBRARY}
		${SDL2_LIBRARY}
		${FREETYPE_LIBRARY}
	)

endif()

###
### No Megasource-specific stuff beyond this point!
###

if(MSVC)
	set(DISABLE_WARNING_FLAG -W0)
else()
	set(DISABLE_WARNING_FLAG -w)
endif()

function(love_disable_warnings ARG_TARGET)
	get_target_property(OLD_FLAGS ${ARG_TARGET} COMPILE_FLAGS)
	set(NEW_FLAGS ${DISABLE_WARNING_FLAG})
	if(OLD_FLAGS)
		set(NEW_FLAGS "${OLD_FLAGS} ${NEW_FLAGS}")
	endif()
	set_target_properties(${ARG_TARGET} PROPERTIES COMPILE_FLAGS ${NEW_FLAGS})
endfunction()

#
# common
#

set(LOVE_SRC_COMMON
	src/common/b64.cpp
	src/common/b64.h
	src/common/Color.h
	src/common/config.h
	src/common/Data.cpp
	src/common/Data.h
	src/common/delay.cpp
	src/common/delay.h
	src/common/EnumMap.h
	src/common/Exception.cpp
	src/common/Exception.h
	src/common/floattypes.cpp
	src/common/floattypes.h
	src/common/int.h
	src/common/math.h
	src/common/Matrix.cpp
	src/common/Matrix.h
	src/common/memory.cpp
	src/common/memory.h
	src/common/Module.cpp
	src/common/Module.h
	src/common/Object.cpp
	src/common/Object.h
	src/common/Optional.h
	src/common/pixelformat.cpp
	src/common/pixelformat.h
	src/common/Stream.cpp
	src/common/Stream.h
	src/common/StringMap.cpp
	src/common/StringMap.h
	src/common/types.cpp
	src/common/types.h
	src/common/utf8.cpp
	src/common/utf8.h
	#src/common/Vector.cpp # Vector.cpp is empty.
	src/common/Vector.h
	src/common/version.h
)

if (APPLE)
	set(LOVE_SRC_COMMON ${LOVE_SRC_COMMON}
		src/common/macosx.mm
	)
endif()

source_group("common" FILES ${LOVE_SRC_COMMON})

#
# love.data
#

set(LOVE_SRC_MODULE_DATA
	src/modules/data/ByteData.cpp
	src/modules/data/ByteData.h
#	src/modules/data/CompressedData.cpp
#	src/modules/data/CompressedData.h
#	src/modules/data/Compressor.cpp
#	src/modules/data/Compressor.h
	src/modules/data/DataModule.cpp
	src/modules/data/DataModule.h
	src/modules/data/DataView.cpp
	src/modules/data/DataView.h
	src/modules/data/HashFunction.cpp
	src/modules/data/HashFunction.h
)

source_group("modules\\data" FILES ${LOVE_SRC_MODULE_DATA})

#
# love.filesystem
#

set(LOVE_SRC_MODULE_FILESYSTEM_ROOT
	src/modules/filesystem/DroppedFile.cpp
	src/modules/filesystem/DroppedFile.h
	src/modules/filesystem/File.cpp
	src/modules/filesystem/File.h
	src/modules/filesystem/FileData.cpp
	src/modules/filesystem/FileData.h
	src/modules/filesystem/Filesystem.cpp
	src/modules/filesystem/Filesystem.h
)

set(LOVE_SRC_MODULE_FILESYSTEM_PHYSFS
	src/modules/filesystem/physfs/File.cpp
	src/modules/filesystem/physfs/File.h
	src/modules/filesystem/physfs/Filesystem.cpp
	src/modules/filesystem/physfs/Filesystem.h
)

set(LOVE_SRC_MODULE_FILESYSTEM
	${LOVE_SRC_MODULE_FILESYSTEM_ROOT}
	${LOVE_SRC_MODULE_FILESYSTEM_PHYSFS}
)

source_group("modules\\filesystem" FILES ${LOVE_SRC_MODULE_FILESYSTEM_ROOT})
source_group("modules\\filesystem\\physfs" FILES ${LOVE_SRC_MODULE_FILESYSTEM_PHYSFS})

#
# love.font
#

set(LOVE_SRC_MODULE_FONT_ROOT
	src/modules/font/BMFontRasterizer.cpp
	src/modules/font/BMFontRasterizer.h
	src/modules/font/Font.cpp
	src/modules/font/Font.h
	src/modules/font/GlyphData.cpp
	src/modules/font/GlyphData.h
	src/modules/font/ImageRasterizer.cpp
	src/modules/font/ImageRasterizer.h
	src/modules/font/Rasterizer.cpp
	src/modules/font/Rasterizer.h
	src/modules/font/TrueTypeRasterizer.cpp
	src/modules/font/TrueTypeRasterizer.h
)

set(LOVE_SRC_MODULE_FONT_FREETYPE
	src/modules/font/freetype/Font.cpp
	src/modules/font/freetype/Font.h
	src/modules/font/freetype/TrueTypeRasterizer.cpp
	src/modules/font/freetype/TrueTypeRasterizer.h
)

set(LOVE_SRC_MODULE_FONT
	${LOVE_SRC_MODULE_FONT_ROOT}
	${LOVE_SRC_MODULE_FONT_FREETYPE}
)

source_group("modules\\font" FILES ${LOVE_SRC_MODULE_FONT_ROOT})
source_group("modules\\font\\freetype" FILES ${LOVE_SRC_MODULE_FONT_FREETYPE})

#
# love.graphics
#

set(LOVE_SRC_MODULE_GRAPHICS_ROOT
	src/modules/graphics/Buffer.cpp
	src/modules/graphics/Buffer.h
	src/modules/graphics/Canvas.cpp
	src/modules/graphics/Canvas.h
	src/modules/graphics/depthstencil.cpp
	src/modules/graphics/depthstencil.h
	src/modules/graphics/Drawable.cpp
	src/modules/graphics/Drawable.h
	src/modules/graphics/Font.cpp
	src/modules/graphics/Font.h
	src/modules/graphics/Graphics.cpp
	src/modules/graphics/Graphics.h
	src/modules/graphics/Image.cpp
	src/modules/graphics/Image.h
	src/modules/graphics/Mesh.cpp
	src/modules/graphics/Mesh.h
	src/modules/graphics/ParticleSystem.cpp
	src/modules/graphics/ParticleSystem.h
	src/modules/graphics/Polyline.cpp
	src/modules/graphics/Polyline.h
	src/modules/graphics/Quad.cpp
	src/modules/graphics/Quad.h
	src/modules/graphics/Resource.h
	src/modules/graphics/Shader.cpp
	src/modules/graphics/Shader.h
	src/modules/graphics/ShaderStage.cpp
	src/modules/graphics/ShaderStage.h
	src/modules/graphics/SpriteBatch.cpp
	src/modules/graphics/SpriteBatch.h
	src/modules/graphics/StreamBuffer.cpp
	src/modules/graphics/StreamBuffer.h
	src/modules/graphics/Text.cpp
	src/modules/graphics/Text.h
	src/modules/graphics/Texture.cpp
	src/modules/graphics/Texture.h
	src/modules/graphics/vertex.cpp
	src/modules/graphics/vertex.h
	src/modules/graphics/Volatile.cpp
	src/modules/graphics/Volatile.h
)

set(LOVE_SRC_MODULE_GRAPHICS_OPENGL
	src/modules/graphics/opengl/Buffer.cpp
	src/modules/graphics/opengl/Buffer.h
	src/modules/graphics/opengl/Canvas.cpp
	src/modules/graphics/opengl/Canvas.h
	src/modules/graphics/opengl/FenceSync.cpp
	src/modules/graphics/opengl/FenceSync.h
	src/modules/graphics/opengl/Graphics.cpp
	src/modules/graphics/opengl/Graphics.h
	src/modules/graphics/opengl/Image.cpp
	src/modules/graphics/opengl/Image.h
	src/modules/graphics/opengl/OpenGL.cpp
	src/modules/graphics/opengl/OpenGL.h
	src/modules/graphics/opengl/Shader.cpp
	src/modules/graphics/opengl/Shader.h
	src/modules/graphics/opengl/ShaderStage.cpp
	src/modules/graphics/opengl/ShaderStage.h
	src/modules/graphics/opengl/StreamBuffer.cpp
	src/modules/graphics/opengl/StreamBuffer.h
)

set(LOVE_SRC_MODULE_GRAPHICS
	${LOVE_SRC_MODULE_GRAPHICS_ROOT}
	${LOVE_SRC_MODULE_GRAPHICS_OPENGL}
)

source_group("modules\\graphics" FILES ${LOVE_SRC_MODULE_GRAPHICS_ROOT})
source_group("modules\\graphics\\opengl" FILES ${LOVE_SRC_MODULE_GRAPHICS_OPENGL})

#
# love.image
#

set(LOVE_SRC_MODULE_IMAGE_ROOT
	src/modules/image/CompressedImageData.cpp
	src/modules/image/CompressedImageData.h
	src/modules/image/CompressedSlice.cpp
	src/modules/image/CompressedSlice.h
	src/modules/image/FormatHandler.cpp
	src/modules/image/FormatHandler.h
	src/modules/image/Image.cpp
	src/modules/image/Image.h
	src/modules/image/ImageData.cpp
	src/modules/image/ImageData.h
	src/modules/image/ImageDataBase.cpp
	src/modules/image/ImageDataBase.h
)

set(LOVE_SRC_MODULE_IMAGE_MAGPIE
	src/modules/image/magpie/ASTCHandler.cpp
	src/modules/image/magpie/ASTCHandler.h
	src/modules/image/magpie/ddsHandler.cpp
	src/modules/image/magpie/ddsHandler.h
	src/modules/image/magpie/KTXHandler.cpp
	src/modules/image/magpie/KTXHandler.h
	src/modules/image/magpie/PKMHandler.cpp
	src/modules/image/magpie/PKMHandler.h
	src/modules/image/magpie/PNGHandler.cpp
	src/modules/image/magpie/PNGHandler.h
	src/modules/image/magpie/PVRHandler.cpp
	src/modules/image/magpie/PVRHandler.h
	src/modules/image/magpie/STBHandler.cpp
	src/modules/image/magpie/STBHandler.h
)

set(LOVE_SRC_MODULE_IMAGE
	${LOVE_SRC_MODULE_IMAGE_ROOT}
	${LOVE_SRC_MODULE_IMAGE_MAGPIE}
)

source_group("modules\\image" FILES ${LOVE_SRC_MODULE_IMAGE_ROOT})
source_group("modules\\image\\magpie" FILES ${LOVE_SRC_MODULE_IMAGE_MAGPIE})

#
# love.math
#

set(LOVE_SRC_MODULE_MATH
	src/modules/math/BezierCurve.cpp
	src/modules/math/BezierCurve.h
	src/modules/math/MathModule.cpp
	src/modules/math/MathModule.h
	src/modules/math/RandomGenerator.cpp
	src/modules/math/RandomGenerator.h
	src/modules/math/Transform.cpp
	src/modules/math/Transform.h
)

source_group("modules\\math" FILES ${LOVE_SRC_MODULE_MATH})

#
# love.thread
#

set(LOVE_SRC_MODULE_THREAD_ROOT
	src/modules/thread/Thread.h
	src/modules/thread/ThreadModule.cpp
	src/modules/thread/ThreadModule.h
	src/modules/thread/threads.cpp
	src/modules/thread/threads.h
)

set(LOVE_SRC_MODULE_THREAD_SDL
	src/modules/thread/sdl/Thread.cpp
	src/modules/thread/sdl/Thread.h
	src/modules/thread/sdl/threads.cpp
	src/modules/thread/sdl/threads.h
)

set(LOVE_SRC_MODULE_THREAD
	${LOVE_SRC_MODULE_THREAD_ROOT}
	${LOVE_SRC_MODULE_THREAD_SDL}
)

source_group("modules\\thread" FILES ${LOVE_SRC_MODULE_THREAD_ROOT})
source_group("modules\\thread\\sdl" FILES ${LOVE_SRC_MODULE_THREAD_SDL})

#
# love.timer
#

set(LOVE_SRC_MODULE_TIMER
	src/modules/timer/Timer.cpp
	src/modules/timer/Timer.h
)

source_group("modules\\timer" FILES ${LOVE_SRC_MODULE_TIMER})

#
# love.window
#

set(LOVE_SRC_MODULE_WINDOW_ROOT
	src/modules/window/Window.cpp
	src/modules/window/Window.h
)

set(LOVE_SRC_MODULE_WINDOW_SDL
	src/modules/window/sdl/Window.cpp
	src/modules/window/sdl/Window.h
)

set(LOVE_SRC_MODULE_WINDOW
	${LOVE_SRC_MODULE_WINDOW_ROOT}
	${LOVE_SRC_MODULE_WINDOW_SDL}
)

source_group("modules\\window" FILES ${LOVE_SRC_MODULE_WINDOW_ROOT})
source_group("modules\\window\\sdl" FILES ${LOVE_SRC_MODULE_WINDOW_SDL})

###################################
# Third-party libraries
###################################

#
# ddsparse
#

set(LOVE_SRC_3P_DDSPARSE
	src/libraries/ddsparse/ddsinfo.h
	src/libraries/ddsparse/ddsparse.cpp
	src/libraries/ddsparse/ddsparse.h
)

add_library(love_3p_ddsparse ${LOVE_SRC_3P_DDSPARSE})

#
# GLAD
#

set(LOVE_SRC_3P_GLAD
	src/libraries/glad/glad.cpp
	src/libraries/glad/glad.hpp
	src/libraries/glad/gladfuncs.hpp
)

add_library(love_3p_glad ${LOVE_SRC_3P_GLAD})

#
# glslang
#

set(LOVE_SRC_3P_GLSLANG_GLSLANG_GENERICCODEGEN
	src/libraries/glslang/glslang/GenericCodeGen/CodeGen.cpp
	src/libraries/glslang/glslang/GenericCodeGen/Link.cpp
)

set(LOVE_SRC_3P_GLSLANG_GLSLANG_INCLUDE
	src/libraries/glslang/glslang/Include/arrays.h
	src/libraries/glslang/glslang/Include/BaseTypes.h
	src/libraries/glslang/glslang/Include/Common.h
	src/libraries/glslang/glslang/Include/ConstantUnion.h
	src/libraries/glslang/glslang/Include/InfoSink.h
	src/libraries/glslang/glslang/Include/InitializeGlobals.h
	src/libraries/glslang/glslang/Include/intermediate.h
	src/libraries/glslang/glslang/Include/PoolAlloc.h
	src/libraries/glslang/glslang/Include/ResourceLimits.h
	src/libraries/glslang/glslang/Include/revision.h
	src/libraries/glslang/glslang/Include/ShHandle.h
	src/libraries/glslang/glslang/Include/Types.h
)

set(LOVE_SRC_3P_GLSLANG_GLSLANG_MACHINEINDEPENDENT_PREPROCESSOR
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/Pp.cpp
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpAtom.cpp
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpContext.cpp
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpContext.h
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpScanner.cpp
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpTokens.cpp
	src/libraries/glslang/glslang/MachineIndependent/preprocessor/PpTokens.h
)

set(LOVE_SRC_3P_GLSLANG_GLSLANG_MACHINEINDEPENDENT
	${LOVE_SRC_3P_GLSLANG_GLSLANG_MACHINEINDEPENDENT_PREPROCESSOR}
	src/libraries/glslang/glslang/MachineIndependent/attribute.cpp
	src/libraries/glslang/glslang/MachineIndependent/attribute.h
	src/libraries/glslang/glslang/MachineIndependent/Constant.cpp
	src/libraries/glslang/glslang/MachineIndependent/gl_types.h
	src/libraries/glslang/glslang/MachineIndependent/glslang_tab.cpp
	src/libraries/glslang/glslang/MachineIndependent/glslang_tab.cpp.h
	src/libraries/glslang/glslang/MachineIndependent/InfoSink.cpp
	src/libraries/glslang/glslang/MachineIndependent/Initialize.cpp
	src/libraries/glslang/glslang/MachineIndependent/Initialize.h
	src/libraries/glslang/glslang/MachineIndependent/Intermediate.cpp
	src/libraries/glslang/glslang/MachineIndependent/intermOut.cpp
	src/libraries/glslang/glslang/MachineIndependent/IntermTraverse.cpp
	src/libraries/glslang/glslang/MachineIndependent/iomapper.cpp
	src/libraries/glslang/glslang/MachineIndependent/iomapper.h
	src/libraries/glslang/glslang/MachineIndependent/limits.cpp
	src/libraries/glslang/glslang/MachineIndependent/linkValidate.cpp
	src/libraries/glslang/glslang/MachineIndependent/LiveTraverser.h
	src/libraries/glslang/glslang/MachineIndependent/localintermediate.h
	src/libraries/glslang/glslang/MachineIndependent/parseConst.cpp
	src/libraries/glslang/glslang/MachineIndependent/ParseContextBase.cpp
	src/libraries/glslang/glslang/MachineIndependent/ParseHelper.cpp
	src/libraries/glslang/glslang/MachineIndependent/ParseHelper.h
	src/libraries/glslang/glslang/MachineIndependent/parseVersions.h
	src/libraries/glslang/glslang/MachineIndependent/pch.cpp
	src/libraries/glslang/glslang/MachineIndependent/pch.h
	src/libraries/glslang/glslang/MachineIndependent/PoolAlloc.cpp
	src/libraries/glslang/glslang/MachineIndependent/propagateNoContraction.cpp
	src/libraries/glslang/glslang/MachineIndependent/propagateNoContraction.h
	src/libraries/glslang/glslang/MachineIndependent/reflection.cpp
	src/libraries/glslang/glslang/MachineIndependent/reflection.h
	src/libraries/glslang/glslang/MachineIndependent/RemoveTree.cpp
	src/libraries/glslang/glslang/MachineIndependent/RemoveTree.h
	src/libraries/glslang/glslang/MachineIndependent/Scan.cpp
	src/libraries/glslang/glslang/MachineIndependent/Scan.h
	src/libraries/glslang/glslang/MachineIndependent/ScanContext.h
	src/libraries/glslang/glslang/MachineIndependent/ShaderLang.cpp
	src/libraries/glslang/glslang/MachineIndependent/SymbolTable.cpp
	src/libraries/glslang/glslang/MachineIndependent/SymbolTable.h
	src/libraries/glslang/glslang/MachineIndependent/Versions.cpp
	src/libraries/glslang/glslang/MachineIndependent/Versions.h
)

set(LOVE_SRC_3P_GLSLANG_GLSLANG_OSDEPENDENT
	src/libraries/glslang/glslang/OSDependent/osinclude.h
)

if(MSVC)
	set(LOVE_SRC_3P_GLSLANG_GLSLANG_OSDEPENDENT
		${LOVE_SRC_3P_GLSLANG_GLSLANG_OSDEPENDENT}
		src/libraries/glslang/glslang/OSDependent/Windows/main.cpp
		src/libraries/glslang/glslang/OSDependent/Windows/ossource.cpp
	)
else()
	set(LOVE_SRC_3P_GLSLANG_GLSLANG_OSDEPENDENT
		${LOVE_SRC_3P_GLSLANG_GLSLANG_OSDEPENDENT}
		src/libraries/glslang/glslang/OSDependent/Unix/ossource.cpp
	)
endif()

set(LOVE_SRC_3P_GLSLANG_GLSLANG_PUBLIC
	src/libraries/glslang/glslang/Public/ShaderLang.h
)

set(LOVE_SRC_3P_GLSLANG_GLSLANG
	${LOVE_SRC_3P_GLSLANG_GLSLANG_GENERICCODEGEN}
	${LOVE_SRC_3P_GLSLANG_GLSLANG_INCLUDE}
	${LOVE_SRC_3P_GLSLANG_GLSLANG_MACHINEINDEPENDENT}
	${LOVE_SRC_3P_GLSLANG_GLSLANG_OSDEPENDENT}
	${LOVE_SRC_3P_GLSLANG_GLSLANG_PUBLIC}
)

set(LOVE_SRC_3P_GLSLANG_OGLCOMPILERSDLL
	src/libraries/glslang/OGLCompilersDLL/InitializeDll.cpp
	src/libraries/glslang/OGLCompilersDLL/InitializeDll.h
)

set(LOVE_SRC_3P_GLSLANG
	${LOVE_SRC_3P_GLSLANG_GLSLANG}
	${LOVE_SRC_3P_GLSLANG_OGLCOMPILERSDLL}
)

add_library(love_3p_glslang ${LOVE_SRC_3P_GLSLANG})

#
# LodePNG
#

set(LOVE_SRC_3P_LODEPNG
	src/libraries/lodepng/lodepng.cpp
	src/libraries/lodepng/lodepng.h
)

add_library(love_3p_lodepng ${LOVE_SRC_3P_LODEPNG})

#
# lz4
#

set(LOVE_SRC_3P_LZ4
	src/libraries/lz4/lz4.c
	src/libraries/lz4/lz4.h
	src/libraries/lz4/lz4hc.c
	src/libraries/lz4/lz4hc.h
	src/libraries/lz4/lz4opt.h
)

add_library(love_3p_lz4 ${LOVE_SRC_3P_LZ4})

#
# noise1234
#

set(LOVE_SRC_3P_NOISE1234
	src/libraries/noise1234/noise1234.cpp
	src/libraries/noise1234/noise1234.h
	src/libraries/noise1234/simplexnoise1234.cpp
	src/libraries/noise1234/simplexnoise1234.h
)

add_library(love_3p_noise1234 ${LOVE_SRC_3P_NOISE1234})

#
# physfs
#

set(LOVE_SRC_3P_PHYSFS
	src/libraries/physfs/physfs_archiver_7z.c
	src/libraries/physfs/physfs_archiver_dir.c
	src/libraries/physfs/physfs_archiver_grp.c
	src/libraries/physfs/physfs_archiver_hog.c
	src/libraries/physfs/physfs_archiver_iso9660.c
	src/libraries/physfs/physfs_archiver_mvl.c
	src/libraries/physfs/physfs_archiver_qpak.c
	src/libraries/physfs/physfs_archiver_slb.c
	src/libraries/physfs/physfs_archiver_unpacked.c
	src/libraries/physfs/physfs_archiver_vdf.c
	src/libraries/physfs/physfs_archiver_wad.c
	src/libraries/physfs/physfs_archiver_zip.c
	src/libraries/physfs/physfs_byteorder.c
	src/libraries/physfs/physfs_casefolding.h
	src/libraries/physfs/physfs_internal.h
	src/libraries/physfs/physfs_lzmasdk.h
	src/libraries/physfs/physfs_miniz.h
	src/libraries/physfs/physfs_platform_haiku.cpp
	src/libraries/physfs/physfs_platform_os2.c
	src/libraries/physfs/physfs_platform_posix.c
	src/libraries/physfs/physfs_platform_qnx.c
	src/libraries/physfs/physfs_platform_unix.c
	src/libraries/physfs/physfs_platform_windows.c
	src/libraries/physfs/physfs_platform_winrt.cpp
	src/libraries/physfs/physfs_platforms.h
	src/libraries/physfs/physfs_unicode.c
	src/libraries/physfs/physfs.c
	src/libraries/physfs/physfs.h
)

if(APPLE)
	set(LOVE_SRC_3P_PHYSFS ${LOVE_SRC_3P_PHYSFS}
		src/libraries/physfs/physfs_platform_apple.m
	)
endif()

add_library(love_3p_physfs ${LOVE_SRC_3P_PHYSFS})

#
# stb_image
#

set(LOVE_SRC_3P_STB
	src/libraries/stb/stb_image.h
)

# stb_image has no implementation files of its own.

#
# utf8
#

set(LOVE_SRC_3P_UTF8_ROOT src/libraries/utf8/utf8.h)

set(LOVE_SRC_3P_UTF8_UTF8
	src/libraries/utf8/utf8/checked.h
	src/libraries/utf8/utf8/core.h
	src/libraries/utf8/utf8/unchecked.h
)

set(LOVE_SRC_3P_UTF8
	${LOVE_SRC_3P_UTF8_ROOT}
	${LOVE_SRC_3P_UTF8_UTF8}
)

# This library is all headers ... so there is no need to
# add_library() here.

#
# Wuff
#

set(LOVE_SRC_3P_WUFF
	src/libraries/Wuff/wuff.c
	src/libraries/Wuff/wuff.h
	src/libraries/Wuff/wuff_config.h
	src/libraries/Wuff/wuff_convert.c
	src/libraries/Wuff/wuff_convert.h
	src/libraries/Wuff/wuff_internal.c
	src/libraries/Wuff/wuff_internal.h
	src/libraries/Wuff/wuff_memory.c
)

add_library(love_3p_wuff ${LOVE_SRC_3P_WUFF})

#
# xxHash
#

set(LOVE_SRC_3P_XXHASH
	src/libraries/xxHash/xxhash.c
	src/libraries/xxHash/xxhash.h
)

add_library(love_3p_xxhash ${LOVE_SRC_3P_XXHASH})

set(LOVE_3P
	love_3p_ddsparse
	love_3p_glad
	love_3p_glslang
	love_3p_lodepng
	love_3p_lz4
	love_3p_noise1234
	love_3p_physfs
	love_3p_wuff
	love_3p_xxhash
)

love_disable_warnings(love_3p_physfs)

#
# liblove
#
set(LOVE_LIB_SRC
	${LOVE_SRC_COMMON}
	# Modules
	${LOVE_SRC_MODULE_AUDIO}
	${LOVE_SRC_MODULE_DATA}
	${LOVE_SRC_MODULE_EVENT}
	${LOVE_SRC_MODULE_FILESYSTEM}
	${LOVE_SRC_MODULE_FONT}
	${LOVE_SRC_MODULE_GRAPHICS}
	${LOVE_SRC_MODULE_IMAGE}
	${LOVE_SRC_MODULE_JOYSTICK}
	${LOVE_SRC_MODULE_KEYBOARD}
	${LOVE_SRC_MODULE_LOVE}
	${LOVE_SRC_MODULE_MATH}
	${LOVE_SRC_MODULE_MOUSE}
	${LOVE_SRC_MODULE_PHYSICS}
	${LOVE_SRC_MODULE_SOUND}
	${LOVE_SRC_MODULE_SYSTEM}
	${LOVE_SRC_MODULE_THREAD}
	${LOVE_SRC_MODULE_TIMER}
	${LOVE_SRC_MODULE_TOUCH}
	${LOVE_SRC_MODULE_VIDEO}
	${LOVE_SRC_MODULE_WINDOW}
)

include_directories(
	src
	src/libraries
	src/modules
	${LOVE_INCLUDE_DIRS}
)

link_directories(${LOVE_LINK_DIRS})

set(LOVE_RC)

if(MSVC)
	set(LOVE_LINK_LIBRARIES ${LOVE_LINK_LIBRARIES}
		ws2_32.lib
		winmm.lib
	)

	set(LOVE_RC
		extra/windows/love.rc
		extra/windows/love.ico
	)
endif()

add_library(${LOVE_LIB_NAME} SHARED ${LOVE_LIB_SRC} ${LOVE_RC})
target_link_libraries(${LOVE_LIB_NAME} ${LOVE_LINK_LIBRARIES} ${LOVE_3P})

if(LOVE_EXTRA_DEPENDECIES)
	add_dependencies(${LOVE_LIB_NAME} ${LOVE_EXTRA_DEPENDECIES})
endif()

if(MSVC)
	set_target_properties(${LOVE_LIB_NAME} PROPERTIES RELEASE_OUTPUT_NAME "love" PDB_NAME "liblove" IMPORT_PREFIX "lib")
	set_target_properties(${LOVE_LIB_NAME} PROPERTIES DEBUG_OUTPUT_NAME "love" PDB_NAME "liblove" IMPORT_PREFIX "lib")
endif()

###################################
# Version
###################################

# Extract version.h contents.
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/src/common/version.h LOVE_VERSION_FILE_CONTENTS)

# Extract one of LOVE_VERSION_MAJOR/MINOR/REV.
function(match_version ARG_STRING OUT_VAR)
	string(REGEX MATCH "VERSION_${ARG_STRING} = ([0-9]+);" TMP_VER "${LOVE_VERSION_FILE_CONTENTS}")
	string(REGEX MATCH "[0-9]+" TMP_VER "${TMP_VER}")
	set(${OUT_VAR} ${TMP_VER} PARENT_SCOPE)
endfunction()

match_version("MAJOR" LOVE_VERSION_MAJOR)
match_version("MINOR" LOVE_VERSION_MINOR)
match_version("REV" LOVE_VERSION_REV)

set(LOVE_VERSION_STR "${LOVE_VERSION_MAJOR}.${LOVE_VERSION_MINOR}")

message(STATUS "Version: ${LOVE_VERSION_STR}")

###################################
# CPack
###################################
install(TARGETS ${LOVE_EXE_NAME} ${LOVE_CONSOLE_EXE_NAME} ${LOVE_LIB_NAME} RUNTIME DESTINATION .)

# Extra DLLs.
if(LOVE_EXTRA_DLLS)
	foreach(DLL ${LOVE_EXTRA_DLLS})
		get_filename_component(DLL_NAME ${DLL} NAME)
		message(STATUS "Extra DLL: ${DLL_NAME}")
	endforeach()
	install(FILES ${LOVE_EXTRA_DLLS} DESTINATION .)
endif()

# Dynamic runtime libs.
if(LOVE_MSVC_DLLS)
	foreach(DLL ${LOVE_MSVC_DLLS})
		get_filename_component(DLL_NAME ${DLL} NAME)
		message(STATUS "Runtime DLL: ${DLL_NAME}")
	endforeach()
	install(FILES ${LOVE_MSVC_DLLS} DESTINATION .)
endif()

# Copy a text file from CMAKE_CURRENT_SOURCE_DIR to CMAKE_CURRENT_BINARY_DIR.
# On Windows, this function will convert line endings to CR,LF.
function(copy_text_file ARG_FILE_IN ARG_FILE_OUT)
	file(READ ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_FILE_IN} TMP_TXT_CONTENTS)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${ARG_FILE_OUT} ${TMP_TXT_CONTENTS})
endfunction()

# Text files.
copy_text_file(readme.md readme.txt)
copy_text_file(license.txt license.txt)
copy_text_file(changes.txt changes.txt)

install(FILES
		${CMAKE_CURRENT_BINARY_DIR}/changes.txt
		${CMAKE_CURRENT_BINARY_DIR}/license.txt
		${CMAKE_CURRENT_BINARY_DIR}/readme.txt
		DESTINATION .)

include(CPack)
