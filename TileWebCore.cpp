#include "TileWebCore.h"

#include <omega/glheaders.h>

TileWebCore* sInstance = NULL;

using namespace Awesomium;

Lock l;

///////////////////////////////////////////////////////////////////////////////
TileWebCore* TileWebCore::instance()
{
    if(!sInstance)
    {
        sInstance = new TileWebCore();
        ModuleServices::addModule(sInstance);
        sInstance->doInitialize(Engine::instance());
    }
    return sInstance;
}

///////////////////////////////////////////////////////////////////////////////
TileWebCore::TileWebCore()
{
    WebConfig wc;
    WebString opt(WSLit("--use-gl=desktop"));
    wc.additional_options.Push(opt);
    //wc.remote_debugging_port = 8080;
    myCore = WebCore::Initialize(wc);

    WebPreferences wp;
    wp.enable_web_gl = true;
    wp.enable_gpu_acceleration = true;
    mySession = myCore->CreateWebSession(WebString((wchar16*)(L"")), wp);

    // Register the local data source with this session.
    myDataSource = new LocalDataSource();
    mySession->AddDataSource(WSLit("local"), myDataSource);
}

///////////////////////////////////////////////////////////////////////////////
TileWebCore::~TileWebCore()
{
    mySession->Release();
    mySession = NULL;

    delete myDataSource;
    myDataSource = NULL;

    // NOTE: WebCore::Shutdown() crashes for some unknown reason. Leaving this
    // commented will cause a leak, but hopefully we are doing this during app
    // shutdown, so it's not too bad.
    //Awesomium::WebCore::Shutdown();
    myCore = NULL;
    sInstance = NULL;
}

///////////////////////////////////////////////////////////////////////////////
void TileWebCore::update(const UpdateContext& context)
{
    l.lock();
    foreach(TileWebRenderPass* rp, myRenderPasses)
    {
        rp->updateOmegaContext(context);
    }
    myCore->Update();
    l.unlock();
}

///////////////////////////////////////////////////////////////////////////////
void TileWebCore::handleEvent(const Event& evt)
{
    // Map mouse input
    if(evt.getServiceType() == Service::Pointer)
    {
        foreach(TileWebRenderPass* rp, myRenderPasses)
        {
            WebView* w = rp->getInternalView();

            const Vector3f& pos = evt.getPosition();

            w->InjectMouseMove(pos[0], pos[1]);

            struct ButtonMap { Event::Flags obtn; MouseButton abtn; };

            static ButtonMap btnmap[] = {
                Event::Left, kMouseButton_Left,
                //Event::Middle, kMouseButton_Middle,
                Event::Right, kMouseButton_Right,
            };

            foreach(ButtonMap& bm, btnmap)
            {
                if(evt.isButtonDown(bm.obtn))
                {
                    w->InjectMouseDown(bm.abtn);
                    //evt.setProcessed();
                }
                else if(evt.isButtonUp(bm.obtn))
                {
                    w->InjectMouseUp(bm.abtn);
                    //evt.setProcessed();
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void TileWebCore::initializeRenderer(Renderer* r)
{
    // Create a view with some standard size. The acutal size will be set inside
    // render
    WebView* v = myCore->CreateWebView(100, 100, mySession);
    v->set_view_listener(this);
    TileWebRenderPass* twrp = new TileWebRenderPass(r, v);
    r->addRenderPass(twrp);
    myRenderPasses.push_back(twrp);
}

///////////////////////////////////////////////////////////////////////////////
void TileWebCore::evaljs(const String& code)
{
    foreach(TileWebRenderPass* twrp, myRenderPasses)
    {
        twrp->getInternalView()->ExecuteJavascript(
            WSLit(code.c_str()), WebString());
    }
}

///////////////////////////////////////////////////////////////////////////////
void TileWebCore::loadUrl(const String& url)
{
    WebURL urlData(WSLit(url.c_str()));
    foreach(TileWebRenderPass* twrp, myRenderPasses)
    {
        twrp->getInternalView()->LoadURL(urlData);
    }
}

////////////////////////////////////////////////////////////////////////////////
void TileWebCore::OnChangeTitle(Awesomium::WebView *caller, const Awesomium::WebString &title)
{
}

////////////////////////////////////////////////////////////////////////////////
void TileWebCore::OnChangeAddressBar(Awesomium::WebView *caller, const Awesomium::WebURL &url)
{
}

////////////////////////////////////////////////////////////////////////////////
void TileWebCore::OnChangeTooltip(Awesomium::WebView *caller, const Awesomium::WebString &tooltip)
{
}

////////////////////////////////////////////////////////////////////////////////
void TileWebCore::OnChangeTargetURL(Awesomium::WebView *caller, const Awesomium::WebURL &url)
{
}

////////////////////////////////////////////////////////////////////////////////
void TileWebCore::OnChangeCursor(Awesomium::WebView *caller, Awesomium::Cursor cursor)
{
}

////////////////////////////////////////////////////////////////////////////////
void TileWebCore::OnChangeFocus(Awesomium::WebView *caller, Awesomium::FocusedElementType focused_type)
{
}

////////////////////////////////////////////////////////////////////////////////
void TileWebCore::OnAddConsoleMessage(Awesomium::WebView *caller, const Awesomium::WebString &message, int line_number, const Awesomium::WebString &source)
{
    ofmsg("%1%(%2%): %3%", %ToString(source) % line_number %ToString(message));
}

////////////////////////////////////////////////////////////////////////////////
void TileWebCore::OnShowCreatedWebView(Awesomium::WebView *caller, Awesomium::WebView *new_view, const Awesomium::WebURL &opener_url, const Awesomium::WebURL &target_url, const Awesomium::Rect &initial_pos, bool is_popup)
{
}

////////////////////////////////////////////////////////////////////////////////
TileWebRenderPass::TileWebRenderPass(Renderer* client, WebView* view) :
    RenderPass(client, "TileWebRenderPass"),
    myView(view),
    myTexture(NULL)
{
    createOmegaContext();
}

///////////////////////////////////////////////////////////////////////////////
void TileWebRenderPass::dispose()
{
}

///////////////////////////////////////////////////////////////////////////////
void TileWebRenderPass::createOmegaContext()
{
    myOmegaContextV = myView->CreateGlobalJavascriptObject(WSLit("omega"));
    myOmegaContext = myOmegaContextV.ToObject();
    myOmegaContext.SetProperty(WSLit("version"), WSLit(OMEGA_VERSION));
}

///////////////////////////////////////////////////////////////////////////////
JSArray Vector3fToJSArray(const Vector3f v)
{
    JSArray a;
    a.Push(JSValue(v[0]));
    a.Push(JSValue(v[1]));
    a.Push(JSValue(v[2]));
    return a;
}

///////////////////////////////////////////////////////////////////////////////
void TileWebRenderPass::updateOmegaContext(const DrawContext& context)
{
    if(context.task == DrawContext::OverlayDrawTask &&
        context.eye == DrawContext::EyeCyclop)
    {
        omicron::Rect& acr = context.tile->activeCanvasRect;
        JSArray jsacr;
        jsacr.Push(JSValue(acr.x()));
        jsacr.Push(JSValue(acr.y()));
        jsacr.Push(JSValue(acr.width()));
        jsacr.Push(JSValue(acr.height()));

        myOmegaContext.SetPropertyAsync(WSLit("activeCanvasRect"), jsacr);
        myOmegaContext.SetPropertyAsync(WSLit("cameraPosition"), Vector3fToJSArray(context.camera->getPosition()));
        myOmegaContext.SetPropertyAsync(WSLit("tileTopLeft"), Vector3fToJSArray(context.tile->topLeft));
        myOmegaContext.SetPropertyAsync(WSLit("tileBottomLeft"), Vector3fToJSArray(context.tile->bottomLeft));
        myOmegaContext.SetPropertyAsync(WSLit("tileBottomRight"), Vector3fToJSArray(context.tile->bottomRight));
    }
    else if(context.task == DrawContext::SceneDrawTask)
    {
        JSArray modelview;
        JSArray projection;

        // The DrawContext modelview is already inverted but we invert it once 
        // more here before passing to javascript, because the Three.js camera 
        // will invert it again, ending with the correct DrawContext.modelview. 
        // We end up with two unneeded inverse computations but there 
        // is no solution for this unless we edit the Three.js codebase.
        AffineTransform3 at = context.modelview.inverse();
        
        for(int i = 0; i < 4; i++)
            for(int j = 0; j < 4; j++)
        {
            modelview.Push(JSValue(at(j, i)));
            projection.Push(JSValue(context.projection(j, i)));
        }
        if(context.eye == DrawContext::EyeCyclop)
        {
            myOmegaContext.SetPropertyAsync(WSLit("modelview"), modelview);
            myOmegaContext.SetPropertyAsync(WSLit("projection"), projection);
        }
        else if(context.eye == DrawContext::EyeRight)
        {
            myOmegaContext.SetPropertyAsync(WSLit("modelviewRight"), modelview);
            myOmegaContext.SetPropertyAsync(WSLit("projectionRight"), projection);
        }
        else if(context.eye == DrawContext::EyeLeft)
        {
            myOmegaContext.SetPropertyAsync(WSLit("modelviewLeft"), modelview);
            myOmegaContext.SetPropertyAsync(WSLit("projectionLeft"), projection);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void TileWebRenderPass::updateOmegaContext(const UpdateContext& context)
{
    myOmegaContext.SetPropertyAsync(WSLit("frame"), JSValue((int)context.frameNum));
    myOmegaContext.SetPropertyAsync(WSLit("time"), JSValue(context.time));
    myOmegaContext.SetPropertyAsync(WSLit("dt"), JSValue(context.dt));
}

///////////////////////////////////////////////////////////////////////////////
void TileWebRenderPass::render(Renderer* client, const DrawContext& context)
{
    l.lock();
    //ofmsg("refc %1%", %myOmegaContext.ref_count());

    if(context.task == DrawContext::OverlayDrawTask && 
        context.eye == DrawContext::EyeCyclop)
    {
        Awesomium::BitmapSurface* surface = static_cast<Awesomium::BitmapSurface*>(myView->surface());
        if(surface)
        {
            // Do we need to resize the web view?
            Vector2i viewSize(surface->width(), surface->height());
            Vector2i vpsize = context.viewport.size();
            if(vpsize != viewSize)
            {
                myView->Resize(vpsize[0], vpsize[1]);
            }
            else
            {
                // If needed, copy pixels from the web surface to our texture.
                if(surface->is_dirty() &&
                   surface->width() == vpsize[0] &&
                   surface->height() == vpsize[1])
                {
                    updateOmegaContext(context);
                    // Let's initialize the texture if we need to.
                    if(myTexture == NULL)
                    {
                        myTexture = context.renderer->createTexture();
                        myTexture->initialize(vpsize[0], vpsize[1], GL_RGBA);
                    }
                    const unsigned char* pixels = surface->buffer();
                    int w = surface->width();
                    int h = surface->height();
                    myTexture->writeRawPixels(surface->buffer(), vpsize[0], vpsize[1], GL_BGRA);
                    surface->set_is_dirty(false);
                }
            }
        }

        // Draw
        if(myTexture)
        {
            DrawInterface* di = context.renderer->getRenderer();
            di->beginDraw2D(context);
            di->drawRectTexture(
                myTexture,
                Vector2f(context.tile->offset[0], context.tile->offset[1]),
                Vector2f(myTexture->getWidth(), myTexture->getHeight()), DrawInterface::FlipY);
            di->endDraw();
        }
    }
    else if(context.task == DrawContext::SceneDrawTask)
    {
        //Awesomium::BitmapSurface* surface = static_cast<Awesomium::BitmapSurface*>(myView->surface());
        //Vector2i vpsize = context.viewport.size();
        //if(surface && surface->width() == vpsize[0] && surface->height() == vpsize[1])
        {
            updateOmegaContext(context);
        }
    }
    l.unlock();
}