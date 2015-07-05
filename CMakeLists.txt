module_version(1.2.1)

# Extract and setup the Awesomium library
set(WEBVIEW_BIN_DIR ${CMAKE_BINARY_DIR}/modules/webView)

# Set module name and source files here
add_library(webView MODULE 
    webView.cpp 
    ClassicWebCore.cpp
    ClassicWebCore.h
    LocalDataSource.cpp
    LocalDataSource.h
    TileWebCore.cpp
    TileWebCore.h)

#--------------------------------------
# Setup Awesomium in windows
if(WIN32)
	execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
		${CMAKE_SOURCE_DIR}/modules/webView/Awesomium-win32.tar.gz WORKING_DIRECTORY ${WEBVIEW_BIN_DIR})
	
	# Copy binary files to the omegalib bin directories.
	set(AWESOMIUM_BIN_DIR ${WEBVIEW_BIN_DIR}/Awesomium-win/build/bin)
	set(AWESOMIUM_BIN_FILES 
		avcodec-53.dll 
		avformat-53.dll 
		avutil-51.dll 
		awesomium.dll 
		awesomium_process.exe 
		icudt.dll 
		libEGL.dll 
		libGLESv2.dll 
		xinput9_1_0.dll)
	foreach( F ${AWESOMIUM_BIN_FILES} )
		configure_file(${AWESOMIUM_BIN_DIR}/${F} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} COPYONLY)
		configure_file(${AWESOMIUM_BIN_DIR}/${F} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE} COPYONLY)
	endforeach()
	
	# Set variables needed to use awesomium
	set(AWESOMIUM_INCLUDE_DIR ${WEBVIEW_BIN_DIR}/Awesomium-win/include)
	set(AWESOMIUM_LIBRARY ${WEBVIEW_BIN_DIR}/Awesomium-win/build/lib/awesomium.lib)

#--------------------------------------
# Setup Awesomium in OSX
elseif(APPLE)

#--------------------------------------
# Setup Awesomium in Linux (32 and 64bit)
elseif(CMAKE_COMPILER_IS_GNUCXX)
	# if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		# set(AWESOMIUM_TGZ ${CMAKE_SOURCE_DIR}/modules/webView/Awesomium-linux32.tar.gz)
		# set(AWESOMIUM_DIR ${WEBVIEW_BIN_DIR}/awesomium_v1.7.2_sdk_linux32)
	# else()
		set(AWESOMIUM_TGZ ${CMAKE_SOURCE_DIR}/modules/webView/Awesomium-linux64.tar.gz)
		set(AWESOMIUM_DIR ${WEBVIEW_BIN_DIR}/awesomium_v1.7.5_sdk_linux64)
#	endif()
	
	execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
		${AWESOMIUM_TGZ} WORKING_DIRECTORY ${WEBVIEW_BIN_DIR})
	# Copy binary files to the omegalib bin directories.
	set(AWESOMIUM_BIN_DIR ${AWESOMIUM_DIR}/bin)
	set(AWESOMIUM_BIN_FILES 
		awesomium_process
		libawesomium-1-7.so.5.0
        libffmpegsumo.so)
	foreach( F ${AWESOMIUM_BIN_FILES} )
		configure_file(${AWESOMIUM_BIN_DIR}/${F} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COPYONLY)
	endforeach()
	
	# Set variables needed to use awesomium
	set(AWESOMIUM_INCLUDE_DIR ${AWESOMIUM_DIR}/include)
	set(AWESOMIUM_LIBRARY ${AWESOMIUM_DIR}/bin/libawesomium-1-7.so.5.0)
    
    # add a symbolic link to awesomium 2.0 (needed at runtime, or apps will
    # not find the library
    add_custom_command(
        TARGET webView POST_BUILD
        COMMAND ln -sf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libawesomium-1-7.so.5.0 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libawesomium-1-7.so.5
    )
endif()

include_directories(${AWESOMIUM_INCLUDE_DIR})

# Set the module library dependencies here
target_link_libraries(webView omega omegaToolkit ${AWESOMIUM_LIBRARY})

#install(FILES ./rift.cfg DESTINATION omegalib/rift)
install(DIRECTORY examples/ DESTINATION omegalib/examples/webView)
install(FILES browser.py DESTINATION omegalib/modules/webView)

declare_native_module(webView)
