from webView import *

ww = WebView.create(1024, 768)
ww.loadUrl("http://github.com")

ui = UiModule.createAndInitialize()
uiroot = ui.getUi()

frame = WebFrame.create(uiroot)
frame.setView(ww)

#getSceneManager().createTexture('web', ww)

#box = BoxShape.create(1,1,1)
#box.setEffect('textured -v emissive -d web')
#box.setPosition(0, 2, -4)