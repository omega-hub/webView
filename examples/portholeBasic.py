# Basic example showing use of the porthole streaming HTML5 interface
from omega import *
from cyclops import *

import porthole

box = BoxShape.create(0.8, 0.8, 0.8)
box.setPosition(Vector3(0, 2, -3))

# Apply an emissive textured effect (no lighting)
box.setEffect("colored -d green")

l1 = Light.create()
l1.setColor(Color('white'))
l1.setPosition(-1, 2, 0)

# Setup porthole
porthole.initialize('colorPicker.xml')
porthole.getService().setConnectedCommand("print('client connected: %id%')")
porthole.getService().setDisconnectedCommand("print('client disconnected: %id%')")

def onColorChanged(id):
	box.getMaterial()

def onPortholeCameraDestroyed(id):
	cam = getCameraById(id)
	getDefaultCamera().removeChild(cam)

# Spin the box!
def onUpdate(frame, t, dt):
	global btn
	box.pitch(dt)
	box.yaw(dt / 3)
	
setUpdateFunction(onUpdate)

def onEvent():
	print(getEvent().getPosition())
setEventFunction(onEvent)

