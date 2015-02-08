# Basic example showing use of the porthole streaming HTML5 interface
from omega import *
from cyclops import *
from webView import *
from browser import *

width = 1200
height = 800

mm = MenuManager.createAndInitialize()
mnu = mm.getMainMenu()
mnu.addButton("Add Browser Tab", "addTab()")

numBrowsers = 1

def addTab():
    global numBrowsers
    ui = UiModule.createAndInitialize()
    uiroot = ui.getUi()
    bw = BrowserWindow('browser{0}'.format(numBrowsers), uiroot, width, height)
    numBrowsers = numBrowsers + 1
    bw.loadUrl("https://stemkoski.github.io/Three.js/HelloWorld.html")
    bw.setDraggable(True)
