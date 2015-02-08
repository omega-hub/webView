#include "ClassicWebCore.h"
#include "TileWebCore.h"

///////////////////////////////////////////////////////////////////////////////
// Python wrapper code.
#ifdef OMEGA_USE_PYTHON
#include "omega/PythonInterpreterWrapper.h"
// Stores the compiled browser module (from browser.py)
PyObject* sBrowserModule = NULL;
BOOST_PYTHON_MODULE(webView)
{
    ///////////////////////////////////////////////////////////////////////////
    // CLASSIC API
    PYAPI_REF_CLASS(WebView, PixelData)
        PYAPI_STATIC_REF_GETTER(WebView, create)
        PYAPI_METHOD(WebView, loadUrl)
        PYAPI_METHOD(WebView, resize)
        PYAPI_METHOD(WebView, setZoom)
        PYAPI_METHOD(WebView, getZoom)
        PYAPI_METHOD(WebView, evaljs)
        ;

    PYAPI_REF_CLASS(WebFrame, Image)
        PYAPI_STATIC_REF_GETTER(WebFrame, create)
        PYAPI_REF_GETTER(WebFrame, getView)
        PYAPI_METHOD(WebFrame, setView)
        ;

    // Compile, load and import the browser module.
    String browserModuleSrc = DataManager::readTextFile("webView/browser.py");
    if(browserModuleSrc != "")
    {
        PyObject* browserModuleCode = Py_CompileString(browserModuleSrc.c_str(), "browser", Py_file_input);
        if(browserModuleCode != NULL)
        {
            const char* moduleName = "browser";
            sBrowserModule = PyImport_ExecCodeModule((char*)moduleName, browserModuleCode);
        }
    }
    else
    {
        owarn("Module webView: could not load extension module browser.py");
    }

    ///////////////////////////////////////////////////////////////////////////
    // TILE API
    PYAPI_REF_BASE_CLASS(TileWebCore)
        PYAPI_STATIC_REF_GETTER(TileWebCore, instance)
        PYAPI_METHOD(TileWebCore, loadUrl)
        PYAPI_METHOD(TileWebCore, evaljs)
        ;
}
#endif

