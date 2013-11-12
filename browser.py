from omega import *
from omegaToolkit import *
from webView import *
from euclid import *

class BrowserWindow():
	view = None
	container = None
	titleBar = None
	frame = None
	draggable = False
	width = 0
	height = 0
	id = "browserWindow"
	
	def __init__(self, id, container, width, height):
		self.width = width
		self.height = height
		self.id = id
		
		self.container = Container.create(ContainerLayout.LayoutVertical, container)
		self.container.setAlpha(1)
		self.titleBar = Label.create(self.container)
		self.titleBar.setText(id)
		self.titleBar.setPinned(True)
		self.titleBar.setDraggable(True)
		self.titleBar.setVisible(False)
		self.titleBar.setAutosize(False)
		self.titleBar.setAlpha(1)
		self.titleBar.setStyleValue('fill', '#000000ff')
		self.titleBar.setHeight(24)
		
		if(isMaster()):
			self.view = WebView.create(width, height)
			self.frame = WebFrame.create(self.container)
			self.frame.setView(self.view)
		else:
			self.view = PixelData.create(width, height, PixelFormat.FormatRgb)
			self.frame = Image.create(self.container)
			self.frame.setData(self.view)

		ImageBroadcastModule.instance().addChannel(self.view, id, ImageFormat.FormatJpeg)
		
	def setDraggable(self, draggable):
		self.draggable = draggable
		self.titleBar.setVisible(draggable)
		
	def loadUrl(self, url):
		if(isMaster()):
			self.view.loadUrl(url)
	
	def resize(self, width, height):
		if(isMaster()):
			self.view.resize(width, height)
		self.frame.setSize(Vector2(width, height))
		# this is a hack ~ the label should resize itself automatically..
		self.titleBar.setWidth(width)
			
