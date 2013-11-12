from webView import *

width = 840
height = 480

ww = None

ui = UiModule.createAndInitialize()
uiroot = ui.getUi()
	
if(isMaster()):
	ww = WebView.create(width, height)
	ww.loadUrl("http://www.yvoschaap.com/chainrxn/")
	frame = WebFrame.create(uiroot)
	frame.setView(ww)
else:
	ww = PixelData.create(width, height, PixelFormat.FormatRgb)
	frame = Image.create(uiroot)
	frame.setData(ww)

ImageBroadcastModule.instance().addChannel(ww, "webpage", ImageFormat.FormatJpeg)
	
