#ifndef __TILE_WEB_CORE__
#define __TILE_WEB_CORE__
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

class TileWebRenderPass;

///////////////////////////////////////////////////////////////////////////////
// Internal Awesomium rendering core and web view manager. User code never
// accesses this directly.
class TileWebCore : public EngineModule, 
                    public Awesomium::WebViewListener::View,
                    public Awesomium::WebViewListener::Process,
                    public Awesomium::JSMethodHandler
{
public:
    static TileWebCore* instance();

    TileWebCore();
    virtual ~TileWebCore();

    virtual void initializeRenderer(Renderer* r);
    void update(const UpdateContext& context);
    void handleEvent(const Event& evt);
    void evaljs(const String& code);
    void loadUrl(const String& url);

    // WebViewListener overrides
    virtual void 	OnChangeTitle(Awesomium::WebView *caller, const Awesomium::WebString &title) {}
    virtual void 	OnChangeAddressBar(Awesomium::WebView *caller, const Awesomium::WebURL &url) {}
    virtual void 	OnChangeTooltip(Awesomium::WebView *caller, const Awesomium::WebString &tooltip) {}
    virtual void 	OnChangeTargetURL(Awesomium::WebView *caller, const Awesomium::WebURL &url) {}
    virtual void 	OnChangeCursor(Awesomium::WebView *caller, Awesomium::Cursor cursor) {}
    virtual void 	OnChangeFocus(Awesomium::WebView *caller, Awesomium::FocusedElementType focused_type) {}
    virtual void 	OnAddConsoleMessage(Awesomium::WebView *caller, const Awesomium::WebString &message, int line_number, const Awesomium::WebString &source);
    virtual void 	OnShowCreatedWebView(Awesomium::WebView *caller, Awesomium::WebView *new_view, const Awesomium::WebURL &opener_url, const Awesomium::WebURL &target_url, const Awesomium::Rect &initial_pos, bool is_popup) {}
    virtual void    OnLaunch(Awesomium::WebView* caller) {}
    virtual void 	OnUnresponsive(Awesomium::WebView *caller);
    virtual void 	OnResponsive(Awesomium::WebView *caller);
    virtual void 	OnCrashed(Awesomium::WebView *caller, Awesomium::TerminationStatus status);
    // JSMethodHandler overrides
    virtual void 	OnMethodCall(Awesomium::WebView *caller, unsigned int remote_object_id, const Awesomium::WebString &method_name, const Awesomium::JSArray &args);
    virtual Awesomium::JSValue 	OnMethodCallWithReturnValue(Awesomium::WebView *caller, unsigned int remote_object_id, const Awesomium::WebString &method_name, const Awesomium::JSArray &args);

private:
    Awesomium::WebCore* myCore;
    Awesomium::WebSession* mySession;
    LocalDataSource* myDataSource;

    List< TileWebRenderPass* > myRenderPasses;
};

///////////////////////////////////////////////////////////////////////////////
class TileWebRenderPass : public RenderPass
{
    friend class TileWebCore;
public:
    TileWebRenderPass(Renderer* client, Awesomium::WebView* view);

    virtual void dispose();
    virtual void render(Renderer* client, const DrawContext& context);

    Awesomium::WebView* getInternalView() { return myView; }

    //! Creates the omegalib javascript context object, used to exchange data
    //! between the omegalib runtime and the webpage.
    void createOmegaContext();
    //! Updates the omegalib javascript context with draw context data.
    void updateOmegaContext(const DrawContext& context);
    //! Updates the omegalib javascript context with update context data.
    void updateOmegaContext(const UpdateContext& context);

    //! Set the frame function name
    void setFrameFunction(const String frameFunction);

private:
    Awesomium::WebView* myView;
    Awesomium::JSValue myWindow;
    Awesomium::JSObject myOmegaContext;
    Awesomium::JSObject myOmegaAPIObject;
    Awesomium::WebString myFrameFunction;
    Ref<Texture> myTexture;
    uint64 myLastWebFrame;
    uint64 myMaxFrameInterval;

    bool myInitialized;
};

#endif