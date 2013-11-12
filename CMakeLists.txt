# Set the module name here
set(MODULE_NAME webView)

# Extract and setup the Awesomium library
set(WEBVIEW_BIN_DIR ${CMAKE_BINARY_DIR}/modules/webView)

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
		set(AWESOMIUM_DIR ${WEBVIEW_BIN_DIR}/awesomium_v1.7.2_sdk_linux64)
#	endif()
	
	execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
		${AWESOMIUM_TGZ} WORKING_DIRECTORY ${WEBVIEW_BIN_DIR})
	# Copy binary files to the omegalib bin directories.
	set(AWESOMIUM_BIN_DIR ${AWESOMIUM_DIR}/bin)
	set(AWESOMIUM_BIN_FILES 
		awesomium_process
		libawesomium-1-7.so.2.0)
	foreach( F ${AWESOMIUM_BIN_FILES} )
		configure_file(${AWESOMIUM_BIN_DIR}/${F} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} COPYONLY)
	endforeach()
	
	# Set variables needed to use awesomium
	set(AWESOMIUM_INCLUDE_DIR ${AWESOMIUM_DIR}/include)
	set(AWESOMIUM_LIBRARY ${AWESOMIUM_DIR}/bin/libawesomium-1-7.so.2.0)
endif()

include_directories(${AWESOMIUM_INCLUDE_DIR})

# Set module name and source files here
add_library(${MODULE_NAME} MODULE 
	webView.cpp)

# Set the module library dependencies here
target_link_libraries(${MODULE_NAME}
	omega omegaToolkit ${AWESOMIUM_LIBRARY})

#install(FILES ./rift.cfg DESTINATION omegalib/rift)

#------------------------------------------------------------------------------
# DO NOT MODIFY ANYTHING BELOW AFTER THIS LINE
set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "")
if(WIN32)
	set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules SUFFIX ".pyd")
endif()
