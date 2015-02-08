#include "TileWebCore.h"

#include <omega/glheaders.h>

TileWebCore* sInstance = NULL;

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
    Awesomium::WebConfig wc;
    Awesomium::WebString opt(Awesomium::WSLit("--use-gl=desktop"));
    wc.additional_options.Push(opt);
    myCore = Awesomium::WebCore::Initialize(wc);

    Awesomium::WebPreferences wp;
    wp.enable_web_gl = true;
    wp.enable_gpu_acceleration = true;
    mySession = myCore->CreateWebSession(Awesomium::WebString((wchar16*)(L"")), wp);

    // Register the local data source with this session.
    myDataSource = new LocalDataSource();
    mySession->AddDataSource(Awesomium::WSLit("local"), myDataSource);
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
    myCore->Update();
}

///////////////////////////////////////////////////////////////////////////////
void TileWebCore::handleEvent(const Event& evt)
{
    // Map mouse input
    if(evt.getServiceType() == Service::Pointer)
    {
        foreach(TileWebRenderPass* rp, myRenderPasses)
        {
            Awesomium::WebView* w = rp->getInternalView();

            const Vector3f& pos = evt.getPosition();

            w->InjectMouseMove(pos[0], pos[1]);

            struct ButtonMap { Event::Flags obtn; Awesomium::MouseButton abtn; };

            static ButtonMap btnmap[] = {
                Event::Left, Awesomium::kMouseButton_Left,
                Event::Middle, Awesomium::kMouseButton_Middle,
                Event::Right, Awesomium::kMouseButton_Right,
            };

            foreach(ButtonMap& bm, btnmap)
            {
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
    Awesomium::WebView* v = myCore->CreateWebView(100, 100, mySession);
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
            Awesomium::WSLit(code.c_str()), Awesomium::WebString());
    }
}

///////////////////////////////////////////////////////////////////////////////
void TileWebCore::loadUrl(const String& url)
{
    Awesomium::WebURL urlData(Awesomium::WSLit(url.c_str()));
    foreach(TileWebRenderPass* twrp, myRenderPasses)
    {
        twrp->getInternalView()->LoadURL(urlData);
    }
}

////////////////////////////////////////////////////////////////////////////////
TileWebRenderPass::TileWebRenderPass(Renderer* client, Awesomium::WebView* view) :
    RenderPass(client, "TileWebRenderPass"),
    myView(view),
    myTexture(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////
void TileWebRenderPass::dispose()
{
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
            //if(context.viewport.size().cwiseNotEqual(viewSize).all())
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
}