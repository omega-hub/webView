#include "ClassicWebCore.h"

#include<Awesomium/STLHelpers.h>

ClassicWebCore* sInstance = NULL;

///////////////////////////////////////////////////////////////////////////////
ClassicWebCore* ClassicWebCore::instance()
{
    if(!sInstance)
    {
        sInstance = new ClassicWebCore();
        ModuleServices::addModule(sInstance);
        sInstance->doInitialize(Engine::instance());
    }
    return sInstance;
}

///////////////////////////////////////////////////////////////////////////////
ClassicWebCore::ClassicWebCore()
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
ClassicWebCore::~ClassicWebCore()
{
    mySession->Release();
    mySession = NULL;

    // NOTE: WebCore::Shutdown() crashes for some unknown reason. Leaving this
    // commented will cause a leak, but hopefully we are doing this during app
    // shutdown, so it's not too bad.
    //Awesomium::WebCore::Shutdown();
    myCore = NULL;
    sInstance = NULL;
}

///////////////////////////////////////////////////////////////////////////////
WebView* ClassicWebCore::createView(int width, int height)
{
    Awesomium::WebView* internalView = myCore->CreateWebView(width, height, mySession);// , Awesomium::kWebViewType_Window);
    WebView* view = new WebView(this, internalView, width, height);
    myViews.push_back(view);
    return view;
}

///////////////////////////////////////////////////////////////////////////////
void ClassicWebCore::destroyView(WebView* view)
{
    oassert(view);
    myViews.remove(view);
}

///////////////////////////////////////////////////////////////////////////////
void ClassicWebCore::update(const UpdateContext& context)
{
    myCore->Update();
    foreach(WebView* view, myViews)
    {
        view->update();
    }
}

///////////////////////////////////////////////////////////////////////////////
WebView* WebView::create(int width, int height)
{
    return ClassicWebCore::instance()->createView(width, height);
}

///////////////////////////////////////////////////////////////////////////////
WebView::WebView(ClassicWebCore* core, Awesomium::WebView* internalView, int width, int height) :
PixelData(PixelData::FormatRgba, width, height, 0),
    myView(internalView),
    myWidth(width),
    myHeight(height)
{
    myView->SetTransparent(true);
}

///////////////////////////////////////////////////////////////////////////////
WebView::~WebView()
{
    if(sInstance != NULL) sInstance->destroyView(this);
    myView = NULL;
}

///////////////////////////////////////////////////////////////////////////////
void WebView::resize(int width, int height)
{
    myWidth = width;
    myHeight = height;
    myView->Resize(width, height);
    PixelData::resize(width, height);
}

///////////////////////////////////////////////////////////////////////////////
void WebView::loadUrl(const String& url)
{
    Awesomium::WebURL urlData(Awesomium::WSLit(url.c_str()));
    myView->LoadURL(urlData);
    myView->Focus();
}

///////////////////////////////////////////////////////////////////////////////
void WebView::update()
{
    Awesomium::BitmapSurface* surface = static_cast<Awesomium::BitmapSurface*>(myView->surface());
    if(surface)
    {
        // Due to threading, webview surface size an pixeldata size may be
        // temporarily out of sync. Check size here to make sure you are not 
        // trashing memory.
        if(surface->width() == myWidth &&
            surface->height() == myHeight)
        {
            unsigned char* ptr = map();
            surface->CopyTo(ptr, myWidth * 4, 4, true, true);
            unmap();
            setDirty();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void WebView::setZoom(int zoom)
{
    myView->SetZoom(zoom);
}

///////////////////////////////////////////////////////////////////////////////
int WebView::getZoom()
{
    return myView->GetZoom();
}

///////////////////////////////////////////////////////////////////////////////
void WebView::evaljs(const String& code)
{
    myView->ExecuteJavascript(Awesomium::WSLit(code.c_str()), Awesomium::WebString());
}

///////////////////////////////////////////////////////////////////////////////
WebFrame* WebFrame::create(Container* container)
{
    WebFrame* frame = new WebFrame(Engine::instance());
    container->addChild(frame);
    return frame;
}

///////////////////////////////////////////////////////////////////////////////
WebFrame::WebFrame(Engine* srv):
    Image(srv)
{
    setEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
void WebFrame::setView(WebView* view)
{
    myView = view;
    setData(myView);
}

///////////////////////////////////////////////////////////////////////////////
void WebFrame::activate()
{
    myView->getInternalView()->Focus();
    const Color& fc = getFactory()->getFocusColor();
    for(int i = 0; i < 4; i++)
    {
        getBorderStyle(i).width = 20;
        getBorderStyle(i).color = fc;
    }
}

///////////////////////////////////////////////////////////////////////////////
void WebFrame::deactivate()
{
    myView->getInternalView()->Unfocus();
    for(int i = 0; i < 4; i++)
    {
        getBorderStyle(i).width = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
void WebFrame::handleEvent(const omega::Event& evt)
{
    Image::handleEvent(evt);
    if(myView != NULL)
    {
        if(isPointerInteractionEnabled())
        {
            if(evt.getServiceType() == Service::Pointer || evt.getServiceType() == Service::Wand)
            {
                Vector2f point  = Vector2f(evt.getPosition().x(), evt.getPosition().y());
                point = transformPoint(point);
                if(simpleHitTest(point))
                {
                    Awesomium::WebView* w = myView->getInternalView();

                    w->InjectMouseMove(point[0], point[1]);
                    if(evt.isButtonDown(UiModule::getClickButton()))		
                    {
                        w->InjectMouseDown(Awesomium::kMouseButton_Left);
                        evt.setProcessed();
                        // Marking local makes events not work on distributed
                        // webframes..
                        //EventSharingModule::markLocal(evt);
                    }
                    else if(evt.isButtonUp(UiModule::getClickButton())) // Need to check buton up like this because mouse service takes away button flag on up button events.
                    {
                        w->InjectMouseUp(Awesomium::kMouseButton_Left);
                        evt.setProcessed();
                        // Marking local makes events not work on distributed
                        // webframes..
                        //EventSharingModule::markLocal(evt);
                    }
                }
            }
        }
        if(evt.getServiceType() == Service::Keyboard)
        {
            if(isActive())
            {
                Awesomium::WebView* w = myView->getInternalView();
                Awesomium::WebKeyboardEvent wke;

                char c;
                if(evt.getChar(&c))
                {

                    if(evt.getType() == Event::Down)
                    {
                        wke.type = Awesomium::WebKeyboardEvent::kTypeChar;
                        wke.text[0] = c;
                        wke.text[1] = 0;
                        w->InjectKeyboardEvent(wke);
                        evt.setProcessed();
                    }
                }
                else
                {
                    if(evt.getType() == Event::Down) wke.type = Awesomium::WebKeyboardEvent::kTypeKeyDown;
                    if(evt.getType() == Event::Up) wke.type = Awesomium::WebKeyboardEvent::kTypeKeyUp;
                    int keys = 0;
                    if(evt.isFlagSet(Event::ButtonLeft)) wke.virtual_key_code = Awesomium::KeyCodes::AK_LEFT;
                    if(evt.isFlagSet(Event::ButtonUp)) wke.virtual_key_code = Awesomium::KeyCodes::AK_UP;
                    if(evt.isFlagSet(Event::ButtonDown)) wke.virtual_key_code = Awesomium::KeyCodes::AK_DOWN;
                    if(evt.isFlagSet(Event::ButtonRight)) wke.virtual_key_code = Awesomium::KeyCodes::AK_RIGHT;
                    if(evt.isFlagSet(Event::Backspace)) wke.virtual_key_code = Awesomium::KeyCodes::AK_BACK;
                    w->InjectKeyboardEvent(wke);
                }
                //else if(evt.getType() == Event::Up) wke.type = Awesomium::WebKeyboardEvent::kTypeKeyUp;
            }
        }
    }

    Image::handleEvent(evt);
}

///////////////////////////////////////////////////////////////////////////////
void WebFrame::update(const omega::UpdateContext& context)
{
    Widget::update(context);
    if(myView != NULL)
    {
        if(myView->getHeight() != getHeight() ||
            myView->getWidth() != getWidth())
        {
            myView->resize(getWidth(), getHeight());
            //setSize(Vector2f(myView->getWidth(), myView->getHeight()));
        }
    }
}
