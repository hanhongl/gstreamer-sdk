cmake_minimum_required(VERSION 2.8)

# determine if jetson-utils is being built as a submodule inside another repo,
# or if it's being build standalone (if the later, we need to do some configuration)
get_directory_property(hasParent PARENT_DIRECTORY)

if(hasParent)
	message("-- CameraSDK:  building as submodule, ${hasParent}")
else()
	message("-- CameraSDK:  building as standalone")
	
	# standalone project
	project(CameraSDK)
  
	# -std=gnu++11
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-write-strings")	
	
	# setup project output paths
	set(PROJECT_OUTPUT_DIR  ${PROJECT_BINARY_DIR}/${CMAKE_SYSTEM_PROCESSOR})
	set(PROJECT_INCLUDE_DIR ${PROJECT_OUTPUT_DIR}/include)

	file(MAKE_DIRECTORY ${PROJECT_INCLUDE_DIR})
	file(MAKE_DIRECTORY ${PROJECT_OUTPUT_DIR}/bin)

	message("-- system arch:  ${CMAKE_SYSTEM_PROCESSOR}")
	message("-- output path:  ${PROJECT_OUTPUT_DIR}")

	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/bin)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/lib)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_OUTPUT_DIR}/lib)
	
	# build C/C++ interface
	include_directories(${PROJECT_INCLUDE_DIR})

endif()


# additional paths for includes and libraries
include_directories(${PROJECT_INCLUDE_DIR}/CameraSDK)
link_directories(/usr/lib/x86_64-linux-gnu /usr/lib/x86_64-linux-gnu/gstreamer-1.0 /usr/lib/aarch64-linux-gnu /usr/lib/aarch64-linux-gnu/gstreamer-1.0)
link_libraries(gstreamer-1.0 gobject-2.0 gstapp-1.0 glib-2.0) 

include_directories(/usr/include/glib-2.0 /usr/lib/x86_64-linux-gnu/glib-2.0/include/ /usr/include/gstreamer-1.0 /usr/lib/x86_64-linux-gnu/gstreamer-1.0/include/ /usr/lib/aarch64-linux-gnu/glib-2.0/include/ /usr/lib/aarch64-linux-gnu/gstreamer-1.0/include/)

# build library
file(GLOB CameraSDKSources *.cpp Base/*.cpp Camera/*.cpp Code/*.cpp)
file(GLOB CameraSDKIncludes *.h *.hpp Base/*.h Camera/*.h Code/*.h)

add_library(CameraSDK SHARED ${CameraSDKSources})

# transfer all headers to the include directory 
file(MAKE_DIRECTORY ${PROJECT_INCLUDE_DIR}/CameraSDK)

foreach(includeFile ${CameraSDKIncludes})
	message("-- Copying ${includeFile}")
	file(RELATIVE_PATH relativeIncludePathFile "${CMAKE_CURRENT_SOURCE_DIR}" "${includeFile}")
	configure_file(${includeFile} ${PROJECT_INCLUDE_DIR}/CameraSDK/${relativeIncludePathFile} COPYONLY)
endforeach()

# build tests/sample executables
#add_subdirectory(Camera)
#add_subdirectory(Code)
#add_subdirectory(Demo/TestCamera)
add_subdirectory(Demo/TestEncode)

