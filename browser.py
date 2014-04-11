from omega import *
from omegaToolkit import *
from webView import *
from euclid import *

browsers = {}

class BrowserWindow():
    view = None
    container = None
    titleBar = None
    frame = None
    draggable = False
    width = 0
    height = 0
    id = "browserWindow"
    parent = None
    
    def __init__(self, id, container, width, height):
        self.parent = container
        self.width = width
        self.height = height
        self.id = id
        
        browsers[id] = self
        
        self.container = Container.create(ContainerLayout.LayoutFree, container)
        self.container.setAlpha(1)
        self.container.setFillColor(Color('black'))
        self.container.setFillEnabled(True)
        self.container.setAutosize(True)
        self.container.setMargin(0)
        # the + 50 is needed to set the right size of the web view (based on scaler size)
        self.container.setSize(Vector2(width + 50, height + 50))
        
        self.titleBar = Container.create(ContainerLayout.LayoutHorizontal, self.container)
        #self.titleBar.setPinned(True)
        #self.titleBar.setDraggable(True)
        self.titleBar.setVisible(False)
        self.titleBar.setAutosize(False)
        self.titleBar.setSizeAnchorEnabled(True)
        self.titleBar.setSizeAnchor(Vector2(0, -1))
        self.titleBar.setPosition(Vector2(80, 0))
        self.titleBar.setAlpha(1)
        self.titleBar.setStyleValue('fill', '#000000ff')
        self.titleBar.setHeight(24)
        #self.titleBar.setWidth(300)
        
        self.label = Label.create(self.container)
        self.label.setText(id)
        self.label.setStyleValue('align', 'middle-left')
        self.label.setPinned(True)
        self.label.setDraggable(True)
        
        self.urlbox = TextBox.create(self.titleBar)
        self.urlbox.setSizeAnchorEnabled(True)
        self.urlbox.setSizeAnchor(Vector2(0, -1))
        self.urlbox.setUIEventCommand('browsers["{0}"].loadUrl("%value%")'.format(id))

        self.close = Button.create(self.titleBar)
        self.close.setText('X')
        self.close.setStyleValue('align', 'middle-right')
        self.close.setUIEventCommand('browsers["{0}"].dispose()'.format(id))
        
        scaler = Container.create(ContainerLayout.LayoutFree, self.container)
        scaler.setDraggable(True)
        scaler.setAutosize(False)
        scaler.setPosition(Vector2(width,height))
        scaler.setSize(Vector2(50,50))
        
        if(isMaster()):
            self.view = WebView.create(width, height)
            self.frame = WebFrame.create(self.container)
            self.frame.setView(self.view)
            self.frame.setSizeAnchorEnabled(True)
            self.frame.setSizeAnchor(Vector2(5, 5))
        # else:
            # self.view = PixelData.create(width, height, PixelFormat.FormatRgb)
            # self.frame = Image.create(self.container)
            # self.frame.setData(self.view)
            
        self.frame.setPosition(Vector2(5, 32))
        self.container.requestLayoutRefresh()
        
        #ImageBroadcastModule.instance().addChannel(self.view, id, ImageFormat.FormatJpeg)
    
    def dispose(self):
        self.parent.removeChild(self.container)
        del browsers[self.id]
    
    def setDraggable(self, draggable):
        self.draggable = draggable
        self.titleBar.setVisible(draggable)
        
    def loadUrl(self, url):
        if(isMaster()):
            self.view.loadUrl(url)
    
    def resize(self, width, height):
        self.width = width
        self.height = height
        if(isMaster()):
            self.view.resize(width, height)
        #self.frame.setSize(Vector2(width, height))
        # this is a hack ~ the label should resize itself automatically..
        self.titleBar.setWidth(width)
            
