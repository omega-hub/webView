webView
=======

HTML5 rendering support for omegalib

## Running on Linux ##
- Make sure you have libudev0 and libgcrypt11 installed
- Set `LD_LIBRARY_PATH` to the omegalib bin directory.


## Tile API ##

### Omegalib / Javascript API ###
Some omegalib functions are available to the page through the `OMEGA` javascript
object.

#### The OMEGA object ####

##### `setFrameFunction(string name)` #####
Sets the name of the function to be called each frame.

#### omegalib context object ####
The omegalib context object is passed to the frame function. It contains the 
following fields.

##### `activeCanvasRect` #####
Array with 4 values with active canvas rect

##### `cameraPosition` #####
3 value array with x,y,z position fo default omegalib camera.

