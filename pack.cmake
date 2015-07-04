if(WIN32)
    file(INSTALL DESTINATION ${PACKAGE_DIR}/bin
        TYPE FILE
        FILES
            ${BIN_DIR}/webView.pyd
            ${BIN_DIR}/avcodec-53.dll
            ${BIN_DIR}/avformat-53.dll
            ${BIN_DIR}/avutil-51.dll
            ${BIN_DIR}/awesomium.dll
            ${BIN_DIR}/awesomium_process.exe
            ${BIN_DIR}/icudt.dll
            #${BIN_DIR}/inspector.pak
            ${BIN_DIR}/libEGL.dll
            ${BIN_DIR}/libGLESv2.dll
            ${BIN_DIR}/xinput9_1_0.dll
        )
elseif(OMEGA_OS_LINUX)
    file(INSTALL DESTINATION ${PACKAGE_DIR}/bin
        TYPE FILE
        FILES
            ${BIN_DIR}/webView.so
        )
endif()

file(INSTALL DESTINATION ${PACKAGE_DIR}/examples/webView
    TYPE DIRECTORY
    FILES
        ${SOURCE_DIR}/modules/webView/examples
    )

file(INSTALL DESTINATION ${PACKAGE_DIR}/modules/webView
    TYPE FILE
    FILES
        ${SOURCE_DIR}/modules/webView/browser.py 
        ${SOURCE_DIR}/modules/webView/tile.py 
    )
    