
# Iota Model Slicer for 3D Color Printing #

## Status: compiling, running, first successful prints

### Overview ###

Iota is a voxel based slicer that can handle textured meshes for 3d printing. Iota can 
generate image slices for inkjet/powder based machines, GCode files for color FDM/FFF 
printing with multiple extruders and mixing extruders, and DXF files for creating stacked 
models with laser cutters.

First ever color printout (XYZPrinting DaVinci Duo, dual extruders)
![picture alt](html/first_color_print.jpg "First ever color printout")

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

* v0.0.7a
  * fixed STL importer to better differentiate binary and ASCII STL files
  * rewrote the mesh fixer to reliably create watertight models every time
  * using half-edges instead of edges to represent triangle meshes
  * remember main window position and size
  * progress bar for slicing
* v0.0.6a
  * https://github.com/MatthiasWM/IotaSlicer/releases/tag/v0.0.6a
  * dxf file writer for slicing on laser cutters (rough code, proven)
  * incremental UI improvements
  * nicer rendering of toolpath extrusion
  * fixes to colored extrusion
* v0.0.5a
  * Iota now compiles on MacOS (Xcode), MSWindows (VisualC 2017), and Ubuntu Linux (Code::Blocks)
* v0.0.4a
  * mostly cleanup around Fluid userinterface design files and versioning
* v0.0.3a 
  * CGode files print successfully on DaVinci Duo printers with Repetier firmware.
* v0.0.2a
  * proof of concept, loading STLs and textures, generating pixel slices
* V0.0.1a
  * initial check-in of concept for a different slicer, aimed at color powder/binder printers

Iota shall be an easy-to-use system for the colorful future of 3D printing.


[MD Markup](https://github.com/tchapi/markdown-cheatsheet/blob/master/README.md)
