project(libGPCAVpp)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake ${CMAKE_MODULE_PATH})

if (CMAKE_SIZEOF_VOID_P MATCHES 8)
	set( PROJECT_ARCH "x86_64" )
else(CMAKE_SIZEOF_VOID_P MATCHES 4)
	set( PROJECT_ARCH "x86" )
endif(CMAKE_SIZEOF_VOID_P MATCHES 8)

set(SOURCE_FILES 
  "src/CodecIDs.cpp"
  "src/Player.cpp"
  "src/Server.cpp"
  "src/Demuxer.cpp" 
  "src/Stream.cpp"
  #"src/Packet.cpp"
  "src/VideoStream.cpp"
  "src/VideoDecoder.cpp"
  "src/DecoderBase.cpp" 
  "src/Error.cpp"
  "src/Frame.cpp"
  "src/VideoFrame.cpp"
  "src/Rational.cpp"
  "src/Muxer.cpp"
  "src/opengl/YUVPainter.cpp"
  "src/opengl/import_opengl.hpp"
  "src/HighResClock.cpp"
  "include/gpc/_av/internal/Decoder.ipp"
)

set(HEADER_FILES 
  "include/gpc/av" 
  "include/gpc/av.hpp"
  "include/gpc/_av/checked_calls.hpp"
  "include/gpc/_av/MediaType.hpp"
  "include/gpc/_av/CodecIDs.hpp"
  "include/gpc/_av/config.hpp"
  "include/gpc/_av/Player.hpp"
  "include/gpc/_av/Server.hpp"
  "include/gpc/_av/Demuxer.hpp"
  "include/gpc/_av/Stream.hpp"
  #"include/gpc/_av/Packet.hpp"
  "include/gpc/_av/VideoStream.hpp"
  "include/gpc/_av/internal/Decoder.hpp"
  "include/gpc/_av/internal/Decoder_Impl.hpp"
  "include/gpc/_av/VideoDecoder.hpp"
  "include/gpc/_av/Error.hpp"
  "include/gpc/_av/Frame.hpp"
  "include/gpc/_av/VideoFrame.hpp"
  "include/gpc/_av/Rational.hpp"
  "include/gpc/_av/internal/HighResClock.hpp"
  "include/gpc/_av/opengl/YUVPainter.hpp"
  "include/gpc/_av/Muxer.hpp"
  "include/gpc/_av/common.hpp"
  "include/gpc/Rational.hpp"  # TODO: move into its own library
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(${PROJECT_NAME} BEFORE
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    #$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
    #$<INSTALL_INTERFACE:include> # TODO
)

# SHADER FILES ------------------------------------------------------

set(SHADER_FILES "src/opengl/vertex.glsl" "src/opengl/fragment.glsl")

source_group("Shader files" FILES ${SHADER_FILES})

# TODO: make this into a CMake module

set(GENERATED "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_include_directories("${PROJECT_NAME}" PUBLIC ${GENERATED})

# Find the GPC Bin2C utility
if (NOT TARGET gpcbin2c)
    find_package(gpcbin2c REQUIRED)
endif()

# Add a custom target and a dependency for each shader file

foreach(shader ${SHADER_FILES})
  get_filename_component(name "${shader}" NAME)
  set(shader_header "${GENERATED}/${name}.h")
  add_custom_command(
    OUTPUT ${shader_header}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader}
    COMMAND gpcbin2c --input=${CMAKE_CURRENT_SOURCE_DIR}/${shader} --output=${shader_header}
  )
  target_sources(${PROJECT_NAME} PRIVATE ${SHADER_FILES} )
  target_sources(${PROJECT_NAME} PRIVATE ${shader_header})
endforeach()

# DEPENDENCIES -------------------------------------------------------

# FFmpeg

if (0)

set(FFmpeg_ROOT "" CACHE PATH "Root of FFmpeg library (libAV)")

foreach (lib "avformat" "avcodec" "avutil" "swscale")

	# Library files
	find_library(${lib}_LIB_RELEASE "${lib}" PATHS ${FFmpeg_ROOT}/lib/${PROJECT_ARCH} ${FFmpeg_ROOT}/lib)
	if (NOT ${lib}_LIB_RELEASE)
		message(FATAL_ERROR "Could not find release version of library \"${lib}\"")
	endif()
	find_library(${lib}_LIB_DEBUG "${lib}" PATH ${FFmpeg_ROOT}/lib/${PROJECT_ARCH} ${FFmpeg_ROOT}/lib)
	if (NOT ${lib}_LIB_DEBUG)
		message(FATAL_ERROR "Could not find debug version of library \"${lib}\"")
	endif()
	target_link_libraries(${PROJECT_NAME} PUBLIC 
		$<$<CONFIG:Release>:${${lib}_LIB_RELEASE}>
		$<$<CONFIG:Debug>:${${lib}_LIB_DEBUG}>
	)

	# Header file
	find_path(${lib}_INCLUDE "lib${lib}/${lib}.h" PATHS ${FFmpeg_ROOT}/include)
	if (NOT ${lib}_INCLUDE)
		message(FATAL_ERROR "Could not find header file of library \"${lib}\"")
	endif()
	target_include_directories(${PROJECT_NAME} PRIVATE ${${lib}_INCLUDE})
	
endforeach()

else()

set(FFmpeg_ROOT "" CACHE PATH "FFMPEG library root directory")

if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(FFmpeg_ARCH "x64")
else()
    set(FFmpeg_ARCH "x86")
endif()

foreach (lib "avformat" "avcodec" "avutil" "swscale")

	# Library files
	find_library(${lib}_LIB_RELEASE "${lib}" PATHS "${FFmpeg_ROOT}" "${FFmpeg_ROOT}/lib/${FFmpeg_ARCH}")
	if (NOT ${lib}_LIB_RELEASE)
		message(FATAL_ERROR "Could not find release version of library \"${lib}\"")
	endif()
	find_library(${lib}_LIB_DEBUG "${lib}d" PATHS "${FFmpeg_ROOT}" "${FFmpeg_ROOT}/lib/${FFmpeg_ARCH}")
	if (NOT ${lib}_LIB_DEBUG)
		#message(FATAL_ERROR "Could not find debug version of library \"${lib}\"")
        #set(${lib}_LIB_DEBUG "${${lib}_LIB_RELEASE}")
		message(WARNING "Could not find debug version of library \"${lib}\", falling back to release version")
	    find_library(${lib}_LIB_DEBUG "${lib}" PATHS "${FFmpeg_ROOT}" "${FFmpeg_ROOT}/lib/${FFmpeg_ARCH}")
	endif()
	target_link_libraries(${PROJECT_NAME} PUBLIC 
		$<$<CONFIG:Release>:${${lib}_LIB_RELEASE}>
		$<$<CONFIG:Debug>:${${lib}_LIB_DEBUG}>
	)

	# Header file
	find_path(${lib}_INCLUDE "lib${lib}/${lib}.h" HINTS "${FFmpeg_ROOT}")
	if (NOT ${lib}_INCLUDE)
		message(FATAL_ERROR "Could not find header file of library \"${lib}\"")
	endif()
	target_include_directories(${PROJECT_NAME} PRIVATE ${${lib}_INCLUDE})
	
endforeach()

endif()

# The following is said to be no longer needed with C++11, which unfortunately
# CMake expressely refuses to detect as such (only individual features) are 
# detected)
target_compile_definitions(${PROJECT_NAME} PRIVATE __STDC_CONSTANT_MACROS)

# If the following does not propagate as it should, the consumer must set the "/SAFESEH:NO" setting by itself
if (MSVC)
    target_link_libraries(${PROJECT_NAME} PUBLIC "-SAFESEH:NO")
endif()

# GLBinding (OpenGL bindings)
if (TARGET GLBinding)
    target_link_libraries(${PROJECT_NAME} PRIVATE GLBinding)
else()
    find_package(GLBinding REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLBINDING_INCLUDE_DIR})
    message("GLBINDING_LIBRARIES:${GLBINDING_LIBRARIES}")
    message("GLBINDING_DEBUG_LIBRARIES:${GLBINDING_DEBUG_LIBRARIES}")
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        $<$<CONFIG:Release>:${GLBINDING_LIBRARIES}>
        $<$<CONFIG:Debug>:${GLBINDING_DEBUG_LIBRARIES}>)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GLBINDING_STATIC)
endif()

# GPC GL utilities
find_package(libGPCGLUtils REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE libGPCGLUtils)

# GPC GL Wrappers
# TODO: this should get pulled in automatically by GPC GL Utils, but there is a problem with CMake there
if (NOT TARGET libGPCGLWrappers)
  find_package(libGPCGLWrappers CONFIG REQUIRED)
endif()
get_target_property(libGPCGLWrappers_INCLUDE_DIRECTORIES libGPCGLWrappers INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(${PROJECT_NAME} PRIVATE "${libGPCGLWrappers_INCLUDE_DIRECTORIES}")

if (0)
# EXPORT && INSTALLATION --------------------------------------------

export(TARGETS ${PROJECT_NAME}  FILE "${PROJECT_NAME}-targets.cmake")
configure_file("PROJECT-config.cmake.in" "${PROJECT_NAME}-config.cmake" @ONLY)
export(PACKAGE ${PROJECT_NAME})

# TODO: install
endif(0)