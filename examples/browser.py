# Basic example showing use of the porthole streaming HTML5 interface
from omega import *
from cyclops import *
from webView import *
from browser import *

width = 400
height = 400

ui = UiModule.createAndInitialize()
uiroot = ui.getUi()

bw = BrowserWindow('browser1', uiroot, width, height)
bw.loadUrl("http://www.google.com")
bw.setDraggable(True)
bw.container.setPosition(Vector2(20,20))
		
scaler = Container.create(ContainerLayout.LayoutFree, bw.container)
scaler.setDraggable(True)
scaler.setAutosize(False)
scaler.setPosition(Vector2(400,400))
scaler.setSize(Vector2(50,50))
#scaler.setStyleValue('fill', 'white')
