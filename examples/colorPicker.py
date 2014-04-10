# Basic example showing use of the porthole streaming HTML5 interface
from omega import *
from cyclops import *
from webView import *
from browser import *

import porthole

box = BoxShape.create(0.8, 0.8, 0.8)
box.setPosition(Vector3(0, 2, -3))

# Apply an emissive textured effect (no lighting)
box.setEffect("colored -d green")

l1 = Light.create()
l1.setColor(Color('white'))
l1.setPosition(-1, 2, 0)

def onColorChanged(colorString):
    box.getMaterial().setColor(Color(colorString), Color('black'))

# Spin the box!
def onUpdate(frame, t, dt):
	global btn
	box.pitch(dt)
	box.yaw(dt / 3)
	
setUpdateFunction(onUpdate)

bw = None

def createWebUi():
	global bw
	# create a webView to show the porthole interface.
	width = 240
	height = 240

	ui = UiModule.createAndInitialize()
	uiroot = ui.getUi()
	
	bw = BrowserWindow('browser', uiroot, width, height)
	bw.loadUrl("http://127.0.0.1:4080")
	bw.setDraggable(True)
		
# Setup porthole
porthole.initialize('colorPicker.xml')
porthole.getService().setConnectedCommand("print('client connected: %id%')")
porthole.getService().setDisconnectedCommand("print('client disconnected: %id%')")
porthole.getService().setServerStartedCommand("createWebUi()")
