
# Iota Model Slicer for 3D Color Printing #

## Status: compiling, running, minimally useful

### Overview ###

Iota is a voxel based slicer that can handle textured meshes for 3d
printing. Iota can generate image slices for inkjet/powder based
machines, and GCode files for color FDM/FFF printing with multiple
extruders and mixing extruders.

### Why Voxels ###

Voxels is short for VOlumetrix piXEL, pixels in 3D space. Iota works by converting 3D vector 
based models into slices of 2D image data. Multiple slices in Z form a complete voxel model.

Current 3D formats only represent the outer shell of 3D objects. Most formats use triangle or a 
superset like polygon meshes or lattices. A common alternative, Constructive Solid Geometry, 
has other limitations. 

Voxels represent the entire volume of models, including inside structures, varying materials
inside a model, colors and transparencies.

### implementation ###

The current code generates somewhate expreimental Gcode files specifically for DaVinci Duo printers.

* v0.0.3a 
   *CGode files print successfully on DaVinci Duo printers with Repetier firmware.

Iota shall be an easy-to-use system for the colorful future of 3D printing.


[MD Markup](https://github.com/tchapi/markdown-cheatsheet/blob/master/README.md)
