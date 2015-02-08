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

#include "LocalDataSource.h"

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

class TileWebRenderPass;

///////////////////////////////////////////////////////////////////////////////
// Internal Awesomium rendering core and web view manager. User code never
// accesses this directly.
class TileWebCore: public EngineModule
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

private:
    Awesomium::WebCore* myCore;
    Awesomium::WebSession* mySession;
    LocalDataSource* myDataSource;

    List< TileWebRenderPass* > myRenderPasses;
};

///////////////////////////////////////////////////////////////////////////////
class TileWebRenderPass : public RenderPass
{
public:
    TileWebRenderPass(Renderer* client, Awesomium::WebView* view);

    virtual void dispose();
    virtual void render(Renderer* client, const DrawContext& context);

    Awesomium::WebView* getInternalView() { return myView; }

private:
    Awesomium::WebView* myView;
    Ref<Texture> myTexture;
};

#endif