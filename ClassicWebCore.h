#ifndef __CLASSIC_WEB_CORE__
#define __CLASSIC_WEB_CORE__
#include <omega.h>
#include <omegaToolkit.h>

#include <Awesomium/STLHelpers.h>
#include <Awesomium/WebKeyboardCodes.h>
#include <Awesomium/WebKeyboardEvent.h>
#include <Awesomium/BitmapSurface.h>
#include <Awesomium/WebCore.h>
#include <Awesomium/WebView.h>
#include <Awesomium/JSObject.h>
#include <Awesomium/WebViewListener.h>

#include "LocalDataSource.h"

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

class WebView;

///////////////////////////////////////////////////////////////////////////////
// Internal Awesomium rendering core and web view manager. User code never
// accesses this directly.
class ClassicWebCore : public EngineModule
{
public:
    static ClassicWebCore* instance();

    ClassicWebCore();
    virtual ~ClassicWebCore();

    WebView* createView(int width, int height);
    void destroyView(WebView*);
    virtual void update(const UpdateContext& context);

private:
    LocalDataSource* myDataSource;
    Awesomium::WebCore* myCore;
    Awesomium::WebSession* mySession;
    List<WebView*> myViews;
};

///////////////////////////////////////////////////////////////////////////////
class WebView: public PixelData
{
public:
    // Static creation function, to keep consistent with the rest of the
    // omegalib python API.
    static WebView* create(int width, int height);

    WebView(ClassicWebCore* core, Awesomium::WebView* internalView, int width, int height);
    virtual ~WebView();
    void resize(int width, int height);
    void update();
    void evaljs(const String& code);

    void loadUrl(const String& url);
    void setZoom(int zoom);
    int getZoom();

    Awesomium::WebView* getInternalView()
    { return myView; }

private:
    Awesomium::WebView* myView;
    int myWidth;
    int myHeight;
};

///////////////////////////////////////////////////////////////////////////////
class WebFrame: public Image
{
public:
    static WebFrame* create(Container* container);
    WebFrame(Engine* srv);

    virtual void handleEvent(const omega::Event& evt);
    virtual void update(const omega::UpdateContext& context);

    void setView(WebView* view);

    WebView* getView() 
    { return myView; }

protected:
    virtual void activate();
    virtual void deactivate();

private:
    Ref<WebView> myView;
};
#endif