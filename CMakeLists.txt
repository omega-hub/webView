# Set the module name here
set(MODULE_NAME webView)

find_path(AWESOMIUM_INCLUDE_DIR Awesomium/awesomium_capi.h)
find_library(AWESOMIUM_LIBRARY Awesomium)


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
