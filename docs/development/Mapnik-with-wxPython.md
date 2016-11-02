<!-- Name: IntegrateWithWxPython -->
<!-- Version: 5 -->
<!-- Last-Modified: 2008/12/13 22:15:55 -->
<!-- Author: victorlin -->


# Mapnik with wxPython

Here's a sample application integrating Mapnik with [wxPython](http://wxpython.org/)

```python
    """
    This is a simple wxPython application demonstrates how to
    integrate mapnik, it do nothing but draw the map from the World Poplulation XML
    example:
    https://github.com/mapnik/mapnik/wiki/GettingStartedInXML
    
    Victor Lin. (bornstub@gmail.com)
    Blog http://blog.ez2learn.com
    
    """
    
    import wx
    import mapnik
    
    class Frame(wx.Frame):
        def __init__(self, *args, **kwargs):
            wx.Frame.__init__(self, size=(800, 500) ,*args, **kwargs)
            self.Bind(wx.EVT_PAINT, self.onPaint)
    
            self.mapfile = "population.xml"
            self.width = 800
            self.height = 500
            self.createMap()
            self.drawBmp()
    
        def createMap(self):
            """Create mapnik object
    
            """
            self.map = mapnik.Map(self.width, self.height)
            mapnik.load_map(self.map, self.mapfile)
            bbox = mapnik.Envelope(mapnik.Coord(-180.0, -75.0), mapnik.Coord(180.0, 90.0))
            self.map.zoom_to_box(bbox)
    
        def drawBmp(self):
            """Draw map to Bitmap object
    
            """
            # create a Image32 object
            image = mapnik.Image(self.width, self.height)
            # render map to Image32 object
            mapnik.render(self.map, image)
            # load raw data from Image32 to bitmap
            self.bmp = wx.BitmapFromBufferRGBA(self.width, self.height, image.tostring())
    
        def onPaint(self, event):
            dc = wx.PaintDC(self)
            memoryDC = wx.MemoryDC(self.bmp)
            # draw map to dc
            dc.Blit(0, 0, self.width, self.height, memoryDC, 0, 0)
    
    if __name__ == '__main__':
        app = wx.App()
        frame = Frame(None, title="wxPython Mapnik Demo By Victor Lin")
        frame.Show()
        app.MainLoop()
```

The key point of this program is this:


```python
    # create a Image32 object
    image = mapnik.Image(self.width, self.height)
    # render map to Image32 object
    mapnik.render(self.map, image)
    # load raw data from Image32 to bitmap
    self.bmp = wx.BitmapFromBufferRGBA(self.width, self.height, image.tostring())
```

We create an Image32 object, and render map on to that image, then load raw data of image into wxPython's Bitmap object. That's it! Now you can use your memory dc to do whatever you like


```python
    memoryDC = wx.MemoryDC(self.bmp)
    # draw map to dc
    dc.Blit(0, 0, self.width, self.height, memoryDC, 0, 0)
```

Screenshots

[[/images/wxMapnikScreenshot.png]]