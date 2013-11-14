from webView import *

width = 840
height = 480

ww = None

ui = UiModule.createAndInitialize()
uiroot = ui.getUi()
	
ww = WebView.create(width, height)
ww.loadUrl("http://www.yvoschaap.com/chainrxn/")
frame = WebFrame.create(uiroot)
frame.setView(ww)
	
