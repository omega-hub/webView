from webView import *

ww = WebView.create(1024, 768)
ww.loadUrl("http://github.com")

ui = UiModule.createAndInitialize()
uiroot = ui.getUi()

frame = WebFrame.create(uiroot)
frame.setView(ww)
