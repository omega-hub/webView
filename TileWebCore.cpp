#include "TileWebCore.h"

#include<Awesomium/STLHelpers.h>

#include <omega/glheaders.h>

#ifndef OMEGA_VERSION
#include <version.h>
#endif

TileWebCore* stwcInstance = NULL;

using namespace Awesomium;

///////////////////////////////////////////////////////////////////////////////
TileWebCore* TileWebCore::instance()
{
    if(!stwcInstance)
    {
        stwcInstance = new TileWebCore();
        ModuleServices::addModule(stwcInstance);
        stwcInstance->doInitialize(Engine::instance());
    }
    return stwcInstance;
}

///////////////////////////////////////////////////////////////////////////////
TileWebCore::TileWebCore()
{
    // We are the last ones processing input (after UI & menu system)
    setPriority(EngineModule::PriorityLowest);

    WebConfig wc;
    WebString opt(WSLit("--use-gl=desktop"));
    wc.additional_options.Push(opt);
    //wc.remote_debugging_port = 8080;
    myCore = WebCore::Initialize(wc);

    WebPreferences wp;
    wp.enable_web_gl = true;
    // Allow cross-domain requests
    wp.enable_web_security = false;
    //wp.enable_gpu_acceleration = true;
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
    stwcInstance = NULL;
}

///////////////////////////////////////////////////////////////////////////////
void TileWebCore::update(const UpdateContext& context)
{
    myCore->Update();

    foreach(TileWebRenderPass* rp, myRenderPasses)
    {
        rp->updateOmegaContext(context);
    }
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
                Event::Middle, kMouseButton_Middle,
                Event::Right, kMouseButton_Right,
            };

            for(int i = 0; i < 3; i++)
            {
                ButtonMap& bm = btnmap[i];
                if(evt.isButtonDown(bm.obtn))
                {
                    w->InjectMouseDown(bm.abtn);
                    evt.setProcessed();
                }
                else if(evt.isButtonUp(bm.obtn))
                {
                    w->InjectMouseUp(bm.abtn);
                    evt.setProcessed();
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
    v->set_process_listener(this);
    // We are going to handle the js callbacks from the omegalib / js API
    // The API is initialized in TileWebRenderPass::createOmegaContext
    v->set_js_method_handler(this);


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
void TileWebCore::OnAddConsoleMessage(Awesomium::WebView *caller, const Awesomium::WebString &message, int line_number, const Awesomium::WebString &source)
{
    ofmsg("%1%(%2%): %3%", %ToString(source) % line_number %ToString(message));
}

////////////////////////////////////////////////////////////////////////////////
void TileWebCore::OnUnresponsive(Awesomium::WebView *caller)
{
    owarn("Webview UNRESPONSIVE");
}

////////////////////////////////////////////////////////////////////////////////
void TileWebCore::OnResponsive(Awesomium::WebView *caller)
{
    owarn("Webview RESPONSIVE");
}

////////////////////////////////////////////////////////////////////////////////
void TileWebCore::OnCrashed(Awesomium::WebView *caller, Awesomium::TerminationStatus status)
{
    owarn("Webview CRASH");
    foreach(TileWebRenderPass* twrp, myRenderPasses) twrp->myInitialized = false;
}

////////////////////////////////////////////////////////////////////////////////
void TileWebCore::OnMethodCall(WebView *caller, unsigned int remote_object_id, const WebString &method_name, const JSArray &args)
{
    if(method_name == WSLit("queueCommand"))
    {
        String f = ToString(args[0].ToString());
        PythonInterpreter* pi = SystemManager::instance()->getScriptInterpreter();
        pi->queueCommand(f);
    }
    else if(method_name == WSLit("setFrameFunction"))
    {
        String f = ToString(args[0].ToString());
        foreach(TileWebRenderPass* twrp, myRenderPasses)
        {
            if(twrp->getInternalView() == caller) twrp->setFrameFunction(f);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
JSValue TileWebCore::OnMethodCallWithReturnValue(WebView *caller, unsigned int remote_object_id, const WebString &method_name, const JSArray &args)
{
    return JSValue();
}

////////////////////////////////////////////////////////////////////////////////
TileWebRenderPass::TileWebRenderPass(Renderer* client, WebView* view) :
    RenderPass(client, "TileWebRenderPass"),
    myView(view),
    myTexture(NULL),
    myLastWebFrame(0),
    myMaxFrameInterval(10),
    myInitialized(false),
    myFrameFunction(WSLit("frame"))
{
    createOmegaContext();
}

///////////////////////////////////////////////////////////////////////////////
void TileWebRenderPass::setFrameFunction(const String frameFunction)
{
    myFrameFunction = ToWebString(frameFunction);
}

///////////////////////////////////////////////////////////////////////////////
void TileWebRenderPass::dispose()
{
}

///////////////////////////////////////////////////////////////////////////////
void TileWebRenderPass::createOmegaContext()
{
    myOmegaContext = JSObject();
    myOmegaContext.SetProperty(WSLit("version"), WSLit(OMEGA_VERSION));

    // Create the omegalib / javascript API object
    myOmegaAPIObject = myView->CreateGlobalJavascriptObject(WSLit("OMEGA")).ToObject();
    myOmegaAPIObject.SetCustomMethod(WSLit("queueCommand"), false);
    myOmegaAPIObject.SetCustomMethod(WSLit("setFrameFunction"), false);
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

        omicron::Rect& ar = context.tile->activeRect;
        JSArray jsar;
        jsar.Push(JSValue(ar.x()));
        jsar.Push(JSValue(ar.y()));
        jsar.Push(JSValue(ar.width()));
        jsar.Push(JSValue(ar.height()));

        Camera* c = context.camera;
        Vector3f cameraEyePosition = c->convertLocalToWorldPosition(c->getHeadOffset());

        myOmegaContext.SetProperty(WSLit("activeCanvasRect"), jsacr);
        myOmegaContext.SetProperty(WSLit("activeRect"), jsar);
        myOmegaContext.SetProperty(WSLit("cameraPosition"), Vector3fToJSArray(cameraEyePosition));
        myOmegaContext.SetProperty(WSLit("tileTopLeft"), Vector3fToJSArray(context.tile->topLeft));
        myOmegaContext.SetProperty(WSLit("tileBottomLeft"), Vector3fToJSArray(context.tile->bottomLeft));
        myOmegaContext.SetProperty(WSLit("tileBottomRight"), Vector3fToJSArray(context.tile->bottomRight));

        // Pass the stereo mode
        int stereoMode = (int)(context.tile->stereoMode != DisplayTileConfig::Default ?
            context.tile->stereoMode :
            context.tile->displayConfig.stereoMode);
        
        if(context.tile->displayConfig.forceMono) stereoMode = DisplayTileConfig::Mono;
            
        myOmegaContext.SetProperty(WSLit("stereoMode"), JSValue(stereoMode));
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
        myOmegaContext.SetProperty(WSLit("modelview"), modelview);
        myOmegaContext.SetProperty(WSLit("projection"), projection);

        if(context.eye == DrawContext::EyeRight)
        {
            myOmegaContext.SetProperty(WSLit("modelviewRight"), modelview);
            myOmegaContext.SetProperty(WSLit("projectionRight"), projection);
        }
        else if(context.eye == DrawContext::EyeLeft)
        {
            myOmegaContext.SetProperty(WSLit("modelviewLeft"), modelview);
            myOmegaContext.SetProperty(WSLit("projectionLeft"), projection);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void TileWebRenderPass::updateOmegaContext(const UpdateContext& context)
{
    myOmegaContext.SetProperty(WSLit("frame"), JSValue((int)context.frameNum));
    myOmegaContext.SetProperty(WSLit("time"), JSValue(context.time));
    myOmegaContext.SetProperty(WSLit("dt"), JSValue(context.dt));

    //if(context.frameNum - myLastWebFrame > myMaxFrameInterval)
    {
        myLastWebFrame = context.frameNum;

        if(!myInitialized)
        {
            bool c = myView->IsCrashed();
            myWindow = myView->ExecuteJavascriptWithResult(WSLit("window"), WSLit(""));
            myInitialized = true;
        }

        JSArray args;
        args.Push(myOmegaContext);

        myWindow.ToObject().Invoke(myFrameFunction, args);
    }
}

///////////////////////////////////////////////////////////////////////////////
void TileWebRenderPass::render(Renderer* client, const DrawContext& context)
{
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
                    // Let's initialize the texture if we need to.
                    if(myTexture == NULL)
                    {
                        myTexture = context.renderer->createTexture();
                        myTexture->initialize(vpsize[0], vpsize[1], 
                            Texture::Type2D,
                            Texture::ChannelRGBA);
                    }
                    const unsigned char* pixels = surface->buffer();
                    int w = surface->width();
                    int h = surface->height();
                    myTexture->writeRawPixels(surface->buffer(), vpsize[0], vpsize[1], GL_BGRA);
                    surface->set_is_dirty(false);
                    myLastWebFrame = 0;
                    updateOmegaContext(context);
                }
            }
        }
    }
    else if(context.task == DrawContext::SceneDrawTask)
    {
        updateOmegaContext(context);
        // Draw
        // NOTE: drawing in one frame behind wrt web page rendering
        // We should put the page rendering here as well, either in the cyclops
        // eye for mono or first eye (left) for stereo.
        if(myTexture)
        {
            const DisplayTileConfig* tile = context.tile;
            float cx = tile->activeCanvasRect.min[0];
            float cy = tile->activeCanvasRect.min[1];
            
            DrawInterface* di = context.renderer->getRenderer();
            di->beginDraw2D(context);
            glColor4f(1,1,1,1);
            di->drawRectTexture(
                myTexture,
                Vector2f(cx, cy),
                Vector2f(myTexture->getWidth(), myTexture->getHeight()), DrawInterface::FlipY);
            di->endDraw();
        }
    }
}