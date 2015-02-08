from webView import *

width = 1200
height = 700

ww = None

ui = UiModule.createAndInitialize()
uiroot = ui.getUi()
	
ww = WebView.create(width, height)
frame = WebFrame.create(uiroot)
frame.setView(ww)
	
