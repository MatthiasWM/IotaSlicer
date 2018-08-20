
# Iota Model Slicer for 3D Color Printing #

## Status: compiling, running, minimally useful

### Overview ###

Iota is a voxel based slicer for colored, textured, and multimaterial 3d prints. 

Currently, Iota generates slices in 2d pixel formats for printing with ink in powder, for 
UV jetting, or screen based SLA.

Vector based output is currently developed to support multicolor and multimaterial 
FDM/FFF printing.

### Why Voxels ###

Voxels is short for VOlumetrix piXEL, pixels in 3D space. Iota works by converting 3D vector 
based models into slices of 2D image data. Multiple slices in Z form a complete voxel model.

Current 3D formats only represent the outer shell of 3D objects. Most formats use triangle or a 
superset like polygon meshes or lattices. A common alternative, Constructive Solid Geometry, 
has other limitations. 

Voxels represent the entire volume of models, including inside structures, varying materials
inside a model, colors and transparencies.

### implementation ###

The current code is merely a cleaned-up proof on concept. The concept seems to work
very well. I am in the process of extending the code base to be able to write useful GCode
from watertight STL meshes. Eventually,

Iota shall be an easy-to-use system for the colorful future of 3D printing.


[MD Markup](https://github.com/tchapi/markdown-cheatsheet/blob/master/README.md)
