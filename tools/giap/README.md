
# GIGNO MAP GENERATOR

Used to convert trenchbroom maps to a format usable by the engine.

## Current workflow :

use 'giap.exe path/to/map/NAME.map' to generate a map from a trenchbroom .map file.
it create 4 things within the 'gigno_engine/assets/maps/' directory:
    - a directory called NAME. Inside this directory :
    - a directiory called 'collisions', in which is a list of numbered .obj model files (one per brush in the map')
    - an empty directory called 'visuals'
    - a gmap file called 'NAME.gmap', containing 3 default entities :
        - a "WorldSpawn" which will automatically spawn the collisions and the visuals in the respective directories
        - a "DirectionalLight"
        - a "GravgunController"

Currently, you need to add the visuals by hand in the 'visuals' directory. To do so, use trenchbroom's export option 'export to obj' and place the resulting model in  the dir.

TODO : I want to add support to place entities in trenchbroom.
