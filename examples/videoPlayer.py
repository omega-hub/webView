from webView import *

width = 560
height = 320

ww = None

ui = UiModule.createAndInitialize()
uiroot = ui.getUi()
	
ww = WebView.create(width, height)
ww.loadUrl('asset://local/webView/examples/video.html')

movieFrame = Container.create(ContainerLayout.LayoutFree, uiroot)
frame = Image.create(movieFrame)
frame.setData(ww)
movieFrame.setAutosize(False)
movieFrame.setSize(Vector2(width, height))

# enable 3d mode for the hud container and attach it to the camera.
c3d = movieFrame.get3dSettings()
c3d.enable3d = True
c3d.position = Vector3(-0.8, 2.5, -2.5)
c3d.scale = 0.003
