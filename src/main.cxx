//
//  IAMesh.cpp
//
//  Copyright (c) 2013-2018 Matthias Melcher. All rights reserved.
//

#define M_MONKEY
//#define M_DRAGON

#include "binaryData.h"

static int kNDrops = 10;

/*

 Slicing Strategy:

 We need two sets of information in a slice.

 Set one defines what is inside and outside of the slice. This is where binder
 needs to go to make the model complete. Missing triangles in non-manifold
 models cause missing layers and broken models. The slicer must not crash
 on missing or deformed triangles. It should instead fill holes in some way.

 Set two calculates the shell color. The color layer is only a few millimeters
 thick to give a deep color, but it must not overlap with other triangles. The
 surface color pixels are extended into the model, following the interopolated
 point normals.

 Slices always have a thickness! The binder slice is generated using two boolean
 operations that cut above and below the slice while maintaining a close
 surface (the resulting slice has a lid at the top and bottom. The slice is then
 rendered from Z to generate the binder voxel layer.

 The color slice is basically a binder slice minus the top and bottom lid, plus
 color and texture information. It is rendered viewing from the top. Only pure
 binder voxels can be changed (no outside voxels, no voxels that were already
 colored).

 To generate a thickness in the color shell, all model vertices are moved by
 half a voxel radius along their negtive point normal, and the color information
 is rendered again. This is repeated until the shell has the required thickness.


 Advanced options:

 We can support known point normals by creating convex and concave surfaces
 that will give a shading similar to what the point-normal-interpolation
 would achieve.

 We can apply bump maps to the model by generating appropriate structures
 in the voxels.


 Support / packaging:

 We can generate support structures for the model while printing it, without
 generating a physical connection to the model. For shipping, the model can then
 be wrapped in a thin foam and placed back in the support structure.


 Dimensions:

 +-V----------+    +-----+        o----> x
 |ooo         |  ==| | | |==      |
 | +--------+ |    |:|:|:|        |     z points up
 > |o       | |    +-----+        V y
 | |        | |
 | |        | |
 | +--------+ |
 | |o       | |
 | |        | |
 | |        | |
 | +--------+ |
 | +--------+ |
 |            |
 +------------+

 physical range rect
 supply box
 build box
 carriage rect
 nozzle rect
 carriage park pos
 roller y pos
 powder collector y pos

 rect: x, y, w, h in relation to end stops
 box: x, y, z, w, h, d in relation to end stops
 pos: x, y

 all measurements are in step of the corresponding axis
 steps per mm are stored seperately


 Test Patterns:

 - Layer height vs. ink per dot
 print 5x5x100mm columns, ten next to each other with 1..10 drops
 print ten layers of that, with smaller layer height every time
 */

/*

 X range is 0...18500, 18500 steps or 512 dots
 y range is 21500...??

 */

/*

 Aug 6th 2017

 - for every slice, we have a zMin and zMax, and a color depth
 - calculate the outline of the zMin slice
 we need the outline to make sure that we have a textured outer surface
 on vertical faces
 - calculate the polygon to fill the zMin slice
 we need the slice polygon to create the infill and possibly a drawing mask
 - clear the drawing area (all transparent)
 use the alpha channel to indicate the amount of binder
 - draw the zMin polygon in infill color
 this is the lower end of the slice
 - render the zMin outline in textured mode
 this is required to make vertical polygons have a minimum thickness
 - set near and far plane and render the entire model in textured mode
 this fills polygons that are not at a vertical angle
 - shrink the geometry by half a pixel and repeat the previous two steps until
 we have the required shell thickness
 (this step may be better in reverse in overdraw mode: inner shell to outer shell)

 * PROBLEM: if structures are smaller than the shell depth, geometry
 shrinking will "flip". One help is to draw a clipping plane with the
 maximum size of the geometry slice. This will not fix self-intersecting
 geometry issues.
 */

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_RGB_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/gl.h>
#include <FL/glu.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "lib3ds.h"

#include "IAMesh.h"
#include "IASlice.h"


#ifdef _MSC_VER
#pragma warning ( disable : 4996 )
#endif

Fl_Slider *zSlider1, *zSlider2;
Fl_RGB_Image *texture = 0L;

IAMeshList gMeshList;
IASlice gMeshSlice;

GLUtesselator *gGluTess = 0;

bool gShowSlice = false;
int gWriteSliceNext = 0;

FILE *gOutFile;

// -----------------------------------------------------------------------------

void writeInt(FILE *f, int32_t x)
{
    uint8_t v;
    // bits 34..28
    if (x&(0xffffffff<<28)) {
        v = ((x>>28) & 0x7f) | 0x80;
        fputc(v, f);
    }
    // bits 27..21
    if (x&(0xffffffff<<21)) {
        v = ((x>>21) & 0x7f) | 0x80;
        fputc(v, f);
    }
    // bits 20..14
    if (x&(0xffffffff<<14)) {
        v = ((x>>14) & 0x7f) | 0x80;
        fputc(v, f);
    }
    // bits 13..7
    if (x&(0xffffffff<<7)) {
        v = ((x>>7) & 0x7f) | 0x80;
        fputc(v, f);
    }
    // bits 6..0
    v = x & 0x7f;
    fputc(v, f);
}

// -----------------------------------------------------------------------------

static int max_vertices = 0;
static int max_texcos = 0;
static int max_normals = 0;


// -----------------------------------------------------------------------------

float minf(float a, float b) { return (a<b)?a:b; }
float maxf(float a, float b) { return (a>b)?a:b; }


void drawModelGouraud()
{
    int i, n = (int)gMeshList.size();
    for (i=0; i<n; i++) {
        IAMesh *IAMesh = gMeshList[i];
        glDepthRange (0.1, 1.0);
        IAMesh->drawGouraud();
    }
}



void drawModelFlat(unsigned int color)
{
    gMeshList.drawFlat(color);
}


void drawModelShrunk(unsigned int color, double d)
{
    int i, n = (int)gMeshList.size();
    for (i=0; i<n; i++) {
        IAMesh *IAMesh = gMeshList[i];
        IAMesh->drawShrunk(color, d);
    }
}

void setShaders() {

    GLuint v, f, p;

    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);

    /*
     vec4 v = vec4(gl_Vertex);
     v.z = 0.0;
     gl_Position = gl_ModelViewProjectionMatrix * v;
     */
    const char * vv =
    "void main(void) {\n"
    "    gl_Position = ftransform();\n"
    "}";

    //  "void main()\n"
    //  "{\n"
    //  "  gl_Position = ftransform();\n"
    //  "}\n"
    //  ;

    // if (pixelIsSilly) dicard;
    const char * ff =
    "void main() {\n"
    "    gl_FragColor = vec4( 1, 1, 0, 1);\n"
    "}"
    ;

    glShaderSource(v, 1, &vv,NULL);
    glShaderSource(f, 1, &ff,NULL);

    glCompileShader(v);
    glCompileShader(f);

    p = glCreateProgram();

    glAttachShader(p,v);
    glAttachShader(p,f);

    glLinkProgram(p);
    glUseProgram(p);
}

void clipToSlice(double z1, double z2)
{
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-66.1,66.1,-66.1,66.1, -z1, -z1-z2); // mm
    glMatrixMode(GL_MODELVIEW);
}

void dontClipToSlice()
{
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-66.1,66.1,-66.1,66.1,-66.1,66.1); // mm
    glMatrixMode(GL_MODELVIEW);
}

class MyGLView : public Fl_Gl_Window
{
    double dx, dy;
public:
    MyGLView(int x, int y, int w, int h, const char *l=0)
    : Fl_Gl_Window(x, y, w, h, l),
    dx(0.0), dy(0.0)
    {
    }
    int handle(int event) {
        static int px = 0, py = 0;
        switch (event) {
            case FL_PUSH:
                px = Fl::event_x();
                py = Fl::event_y();
                return 1;
            case FL_DRAG:
            case FL_RELEASE:
                dx = dx + (px - Fl::event_x())*0.3;
                dy = dy + (py - Fl::event_y())*0.3;
                px = Fl::event_x();
                py = Fl::event_y();
                redraw();
                return 1;
        }
        return Fl_Gl_Window::handle(event);
    }

    void draw(IAMeshList *meshList, IASlice *meshSlice)
    {
        double z1 = zSlider1->value();
        double z2 = zSlider2->value();
        //---- draw the model using the near and far plane for clipping
        if (meshList) {
            clipToSlice(z1, z2);
#if 0
            glDisable(GL_TEXTURE_2D);
            meshList->drawFlat(FL_RED);
            glDisable(GL_TEXTURE_2D);
#else
            glEnable(GL_TEXTURE_2D);
            meshList->drawFlat(FL_WHITE);
            glDisable(GL_TEXTURE_2D);
#endif
        }

        //---- draw the lid outline
        if (meshSlice) {
            dontClipToSlice();
#if 0
            glDisable(GL_TEXTURE_2D);
            glColor3ub(128, 255, 255);
            meshSlice->drawLidEdge();
            glDisable(GL_TEXTURE_2D);
#else
            glEnable(GL_TEXTURE_2D);
            glColor3ub(255, 255, 255);
            meshSlice->drawLidEdge();
            glDisable(GL_TEXTURE_2D);
#endif
        }

        dontClipToSlice();
    }

    void draw()
    {
        static bool firstTime = true;
        if (firstTime) {
            firstTime = false;
            //      setShaders();
        }

        if (!valid()) {
            static GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
            static GLfloat mat_shininess[] = { 50.0 };
            //static GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
            static GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
            static GLfloat light_ambient[] = { 0.3, 0.3, 0.3, 1.0};

            gl_font(FL_HELVETICA_BOLD, 16 );

            glClearColor (0.0, 0.0, 0.0, 0.0);
            glShadeModel (GL_SMOOTH);

            glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
            glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
            glLightfv(GL_LIGHT0, GL_POSITION, light_position);
            glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);

            glEnable(GL_BLEND);
            //      glBlendFunc(GL_ONE, GL_ZERO);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glViewport(0,0,w(),h());

            if (texture) {
                static GLuint tex = 0;
                glGenTextures(1, &tex);
                glBindTexture(GL_TEXTURE_2D, tex);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture->w(), texture->h(),
                             0, GL_RGB, GL_UNSIGNED_BYTE, *texture->data());
                glEnable(GL_TEXTURE_2D);
            }
        }

        double z1 = zSlider1->value();
        double z2 = zSlider2->value();

        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();
        if (gShowSlice) {
            glOrtho(-66.1,66.1,-66.1,66.1, -z1, -z1-z2); // mm
        } else {
            glOrtho(-66.1,66.1,-66.1,66.1,-66.1,66.1); // mm
        }
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPushMatrix();

        if (gShowSlice) {
            // show just the slice
            glDisable(GL_LIGHTING);
            glDisable(GL_DEPTH_TEST);

            //---- draw the untextured lid surface
            // change the z range to disable clipping
            dontClipToSlice();
#if 0
            gMeshSlice.drawFlat(FL_GREEN);
#else
            gMeshSlice.drawFlat(FL_DARK3);
#endif

            for (int n = 10; n>0; --n) {
                gMeshList.shrinkTo(0.2*n);
                IASlice meshSlice;
                meshSlice.generateFrom(gMeshList, zSlider1->value());
                draw(&gMeshList, &meshSlice);
            }
            gMeshList.shrinkTo(0.0);

            draw(&gMeshList, &gMeshSlice);

#if 0
            // set the z range again to enable drawing the shell
            glMatrixMode (GL_PROJECTION);
            glLoadIdentity();
            glOrtho(-66.1,66.1,-66.1,66.1, -z1, -z1-z2); // mm
            glMatrixMode(GL_MODELVIEW);
            // the following code guarantees a hull of at least 1mm width
            //      double sd;
            //      glHint(GL_POLYGON_SMOOTH_HINT, );
            //      glDisable (GL_POLYGON_SMOOTH);
            //      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            //      for (sd = 0.2; sd<gMinimumShell; sd+=0.2) {
            //        drawModelShrunk(Fl_WHITE, sd);
            //      }
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
        } else {
            // show the 3d model
            glRotated(-dy, 1.0, 0.0, 0.0);
            glRotated(-dx, 0.0, 1.0, 0.0);
            glEnable(GL_LIGHTING);
            glEnable(GL_DEPTH_TEST);
            drawModelGouraud();
        }
        glPopMatrix();

        if (gWriteSliceNext==1) {
            gWriteSliceNext = 0;
            writeSlice();
        } else if (gWriteSliceNext==2) {
            gWriteSliceNext = 0;
            writePrnSlice();
        }

        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, w(), 0, h(), -10, 10); // mm
        glMatrixMode(GL_MODELVIEW);
        gl_color(FL_WHITE);
        char buf[1024];
        sprintf(buf, "Slice at %.4gmm", z1); gl_draw(buf, 10, 40);
        sprintf(buf, "%.4gmm thick", z2); gl_draw(buf, 10, 20);
    }

    void writeSlice(int nDrops = kNDrops, int interleave=4) {
        printf("# slice at %gmm\n", zSlider1->value());
        // 500 high at 12 pixels = 41 swashes.
        // 500 high at 12 pixels, interleave 4 = 166 swashes.
        int incr = 12/interleave;
        int x, i, n = (h()-11)/incr, ww = w(), row;
        for (i=0; i<n; i++) {
            int nLeft = 0, nFill = 0, nRight = 0;
            printf("Swash %3d: ", i);
            uint32_t *buf = (uint32_t*)malloc(ww*12*4);
            glReadPixels(0, i*incr, ww, 12, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buf);
            // find the first pixel
            for (x=0; x<ww; x++) {
                for (row=0; row<12; row++) {
                    if (buf[x+row*ww]!=0) break;
                }
                if (row<12) break;
            }
            if (x<ww) {
                nLeft = x;
                for (x=ww-1; x>nLeft; x--) {
                    for (row=0; row<12; row++) {
                        if (buf[x+row*ww]!=0) break;
                    }
                    if (row<12) break;
                }
                nRight = ww-x;
                nFill = ww-nLeft-nRight;
                printf("%d / %d /%d", nLeft, nFill, nRight);
                //fprintf(f, "# swash %d\n", i);
#if 0
                int rpt;
                for (rpt = 0; rpt<1; rpt++) {
                    // yGoto
                    writeInt(gOutFile, 147);
                    writeInt(gOutFile, 25536+425*i); // swash height (428)
                    // xGoto
                    writeInt(gOutFile, 144);
                    writeInt(gOutFile, 100+36*nLeft); // first pixel
                    for (x=nLeft; x<nLeft+nFill; x++) {
                        uint32_t v = 0;
                        for (row=0; row<12; row++) {
                            v = v<<1;
                            if (buf[x+row*ww]!=0) v |= 1;
                        }
                        // fire pattern
                        writeInt(gOutFile, 1);
                        writeInt(gOutFile, v);
                    }
                }
#else
                // yGoto
                writeInt(gOutFile, 147);
                writeInt(gOutFile, 22000+425*i*incr/12); // swash height (428)
                // xGoto
                writeInt(gOutFile, 144);
                writeInt(gOutFile, 100+36*nLeft); // first pixel
                for (x=nLeft; x<nLeft+nFill; x++) {
                    uint32_t v = 0;
                    for (row=0; row<12; row++) {
                        v = v<<1;
                        if (buf[x+row*ww]!=0) v |= 1;
                    }
                    // fire pattern times n
                    writeInt(gOutFile, 2);
                    writeInt(gOutFile, v);
                    writeInt(gOutFile, nDrops);
                }
#endif
            } else {
                printf("empty");
            }
            free(buf);
            printf("\n");
        }
    }

    /*
     720 dpi
     1440 dpi
     2128 bpl (1064 samples) 3"??
     11904 hgt = 8.2"
     8501 wdt = 11" ?? 27,94cm
     0.299896 paper  paperWdt = wdt / xdpi * 0.0254
     5 colors

     126658592 bytes  size = hgt * bpl * colors!
     */

    void writePrnSlice() {
        printf("# slice at %gmm\n", zSlider1->value());
        char name[2048];
        sprintf(name, "/Users/matt/prn/f%05d.prn", (int)((zSlider1->value()+20)*10));
        FILE *f = fopen(name, "wb");
        // print the header:
        int x, i, n = h(), ww = w(), w2 = 8501;
        struct hdr {
            uint32_t sig, xdpi, ydpi, bpl, hgt, wdt;
            float paperWidth;
            uint32_t nColors, r0, r1, r2, r3;
        };
        struct hdr hdr = {
            0x5555, 720, 1440, (uint32_t)(w()*2), (uint32_t)h()*2, (uint32_t)w2,
            w2/720.0f*0.0254f,
            4, 2, 8, 2, 0 };
        fwrite(&hdr, sizeof(hdr), 1, f);
        // print the color data:
        uint16_t blk = 0xffff, wht = 0x0000;
        for (i=0; i<n; i++) {
            printf("Swash %3d: ", i);
            uint32_t *buf = (uint32_t*)malloc(ww*1*4);
            glReadPixels(0, i, ww, 1, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buf);
            int sw;
            for (sw=0; sw<2; sw++) {
                // C
                for (x=0; x<ww; x++) {
                    fwrite(&wht, sizeof(wht), 1, f);
                }
                // M
                for (x=0; x<ww; x++) {
                    fwrite(&wht, sizeof(wht), 1, f);
                }
                // Y
                for (x=0; x<ww; x++) {
                    fwrite(&wht, sizeof(wht), 1, f);
                }
                // black
                for (x=0; x<ww; x++) {
                    if (buf[x+ww]!=0) {
                        fwrite(&blk, sizeof(blk), 1, f);
                    } else {
                        fwrite(&wht, sizeof(wht), 1, f);
                    }
                }
            }
            free(buf);
            printf("\n");
        }
    }

};

MyGLView *glView = 0L;

double minX = 0.0, maxX = 0.0, minY = 0.0, maxY = 0.0, minZ = 0.0, maxZ = 0.0;

static double min(double a, double b) { return a<b?a:b; }
static double max(double a, double b) { return a>b?a:b; }

/**
 Load a single node from a 3ds file.
 */
void load3ds(Lib3dsFile *f, Lib3dsMeshInstanceNode *node) {
    float (*orig_vertices)[3];
    int export_texcos;
    int export_normals;
    int i;
    Lib3dsMesh *mesh;

    IAMesh *msh = new IAMesh();
    gMeshList.push_back(msh);

    mesh = lib3ds_file_mesh_for_node(f, (Lib3dsNode*)node);
    if (!mesh || !mesh->vertices) return;

    orig_vertices = (float(*)[3])malloc(sizeof(float) * 3 * mesh->nvertices);
    memcpy(orig_vertices, mesh->vertices, sizeof(float) * 3 * mesh->nvertices);
    {
        float inv_matrix[4][4], M[4][4];
        float tmp[3];
        int i;

        lib3ds_matrix_copy(M, node->base.matrix);
        lib3ds_matrix_translate(M, -node->pivot[0], -node->pivot[1], -node->pivot[2]);
        lib3ds_matrix_copy(inv_matrix, mesh->matrix);
        lib3ds_matrix_inv(inv_matrix);
        lib3ds_matrix_mult(M, M, inv_matrix);

        //lib3ds_matrix_rotate(M, 90, 1, 0, 0);

        for (i = 0; i < mesh->nvertices; ++i) {
            lib3ds_vector_transform(tmp, M, mesh->vertices[i]);
            lib3ds_vector_copy(mesh->vertices[i], tmp);
        }
    }
    {
        int i;
        for (i = 0; i < mesh->nvertices; ++i) {
            IAVertex *isPoint = new IAVertex();
            isPoint->pPosition.read(mesh->vertices[i]);
            minX = min(minX, isPoint->pPosition.x());
            maxX = max(maxX, isPoint->pPosition.x());
            minY = min(minY, isPoint->pPosition.y());
            maxY = max(maxY, isPoint->pPosition.y());
            minZ = min(minZ, isPoint->pPosition.z());
            maxZ = max(maxZ, isPoint->pPosition.z());
            //isPoint->pPosition *= 10;
            //isPoint->pPosition *= 40; // mokey full size
            //isPoint->pPosition *= 5; // mokey tiny (z=-5...+5)
#ifdef M_MONKEY
            isPoint->pTex.set(
                              isPoint->pPosition.x() * 0.8 + 0.5,
                              -isPoint->pPosition.y() * 0.8 + 0.5,
                              0.0
                              );
            //isPoint->pPosition *= 10; // mokey tiny (z=-5...+5)
            isPoint->pPosition *= 30; // mokey tiny (z=-5...+5)
            isPoint->pInitialPosition = isPoint->pPosition;
#elif defined M_DRAGON
            isPoint->pPosition *= 1; // dragon (z=-5...+5)
#elif defined M_PEPSI
            isPoint->pPosition *= 1; // pepsi (z=-5...+5)
#endif
            msh->vertexList.push_back(isPoint);
        }
    }

    printf("Model bounding box is:\n  x: %g, %g\n  y: %g, %g\n  z: %g, %g\n",
           minX, maxX, minY, maxY, minZ, maxZ);
    /*
     Monkey Model bounding box is:
     x: -1.36719, 1.36719
     y: -0.984375, 0.984375
     z: -0.851562, 0.851562
     Iota wants mm, so scale by 40, resulting in a 112mm wide head.
     */

    export_texcos = (mesh->texcos != 0);
    export_normals = (mesh->faces != 0);

    //  for (i = 0; i < mesh->nvertices; ++i) {
    //    fprintf(o, "v %f %f %f\n", mesh->vertices[i][0],
    //            mesh->vertices[i][1],
    //            mesh->vertices[i][2]);
    //  }
    //  fprintf(o, "# %d vertices\n", mesh->nvertices);

    //  if (export_texcos) {
    //    for (i = 0; i < mesh->nvertices; ++i) {
    //      fprintf(o, "vt %f %f\n", mesh->texcos[i][0],
    //              mesh->texcos[i][1]);
    //    }
    //    fprintf(o, "# %d texture vertices\n", mesh->nvertices);
    //  }

    //  if (export_normals) {
    //    float (*normals)[3] = (float(*)[3])malloc(sizeof(float) * 9 * mesh->nfaces);
    //    lib3ds_mesh_calculate_vertex_normals(mesh, normals);
    //    for (i = 0; i < 3 * mesh->nfaces; ++i) {
    //      fprintf(o, "vn %f %f %f\n", normals[i][0],
    //              normals[i][1],
    //              normals[i][2]);
    //    }
    //    free(normals);
    //    fprintf(o, "# %d normals\n", 3 * mesh->nfaces);
    //  }

    {
        //    int mat_index = -1;
        for (i = 0; i < mesh->nfaces; ++i) {

            //      if (mat_index != mesh->faces[i].material) {
            //        mat_index = mesh->faces[i].material;
            //        if (mat_index != -1) {
            //          fprintf(o, "usemtl %s\n", f->materials[mat_index]->name);
            //        }
            //      }
            //      fprintf(o, "f ");
            //      for (j = 0; j < 3; ++j) {
            IATriangle *t = new IATriangle();
            t->pVertex[0] = msh->vertexList[mesh->faces[i].index[0]];
            t->pVertex[1] = msh->vertexList[mesh->faces[i].index[1]];
            t->pVertex[2] = msh->vertexList[mesh->faces[i].index[2]];
            //      IATriangle->print();
            msh->addFace(t);
            //        fprintf(o, "%d", mesh->faces[i].index[j] + max_vertices + 1);
            //        int vi;
            //        float *fv;
            //        vi = mesh->faces[i].index[j];
            //        fv = mesh->vertices[vi];
            //        glVertex3fv(fv);
            //        if (export_texcos) {
            //          fprintf(o, "/%d", mesh->faces[i].index[j] + max_texcos + 1);
            //        } else if (export_normals) {
            //          fprintf(o, "/");
            //        }
            //        if (export_normals) {
            //          fprintf(o, "/%d", 3 * i + j + max_normals + 1);
            //        }
            //        if (j < 3) {
            //          fprintf(o, " ");
            //        }
            //      }
            //      fprintf(o, "\n");
        }
    }

    max_vertices += mesh->nvertices;
    if (export_texcos)
        max_texcos += mesh->nvertices;
    if (export_normals)
        max_normals += 3 * mesh->nfaces;

    memcpy(mesh->vertices, orig_vertices, sizeof(float) * 3 * mesh->nvertices);
    free(orig_vertices);

    msh->validate();
    // TODO: fix seams
    // TODO: fix zero size holes
    // TODO: fix degenrate triangles
    msh->fixHoles();
    msh->validate();

    msh->clearNormals();
    msh->calculateNormals();
}


int getShort(FILE *f) {
    int ret = 0;
    ret |= fgetc(f);
    ret |= fgetc(f)<<8;
    return ret;
}

int getInt(FILE *f) {
    int ret = 0;
    ret |= fgetc(f);
    ret |= fgetc(f)<<8;
    ret |= fgetc(f)<<16;
    ret |= fgetc(f)<<24;
    return ret;
}

float getFloat(FILE *f) {
    float ret;
    fread(&ret, 4, 1, f);
    return ret;
}

int addPoint(IAMesh *IAMesh, float x, float y, float z)
{
    int i, n = (int)IAMesh->vertexList.size();
    for (i = 0; i < n; ++i) {
        IAVertex *v = IAMesh->vertexList[i];
        if (   v->pPosition.x()==x
            && v->pPosition.y()==y
            && v->pPosition.z()==z)
        {
            return i;
        }
    }
    IAVertex *v = new IAVertex();
    v->pPosition.set(x, y, z);
    IAMesh->vertexList.push_back(v);
    return n;
}

/**
 Load a single node from a binary stl file.
 */
void loadStl(const char *filename) {
    int i;

    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "ERROR openening file!\n");
        return;
    }
    fseek(f, 0x50, SEEK_SET);
    IAMesh *msh = new IAMesh();
    gMeshList.push_back(msh);

    int nFaces = getInt(f);
    for (i=0; i<nFaces; i++) {
        float x, y, z;
        int p1, p2, p3;
        // face normal
        getFloat(f);
        getFloat(f);
        getFloat(f);
        // point 1
        x = getFloat(f);
        y = getFloat(f);
        z = getFloat(f);
        p1 = addPoint(msh, x, y, z);
        // point 2
        x = getFloat(f);
        y = getFloat(f);
        z = getFloat(f);
        p2 = addPoint(msh, x, y, z);
        // point 3
        x = getFloat(f);
        y = getFloat(f);
        z = getFloat(f);
        p3 = addPoint(msh, x, y, z);
        // add face
        IATriangle *t = new IATriangle();
        t->pVertex[0] = msh->vertexList[p1];
        t->pVertex[1] = msh->vertexList[p2];
        t->pVertex[2] = msh->vertexList[p3];
        msh->addFace(t);
        // color
        getShort(f);
    }

    msh->validate();
    // TODO: fix seams
    // TODO: fix zero size holes
    // TODO: fix degenrate triangles
    msh->fixHoles();
    msh->validate();

    msh->clearNormals();
    msh->calculateNormals();

    fclose(f);
}





/**
 Load a model from a 3ds file.
 */
void load3ds(const char *filename)
{
    Lib3dsFile *f = lib3ds_file_open(filename);
    if (!f->nodes)
        lib3ds_file_create_nodes_for_meshes(f);
    lib3ds_file_eval(f, 0);
    Lib3dsNode *p;
    for (p = f->nodes; p; p = p->next) {
        if (p->type == LIB3DS_NODE_MESH_INSTANCE) {
            load3ds(f, (Lib3dsMeshInstanceNode*)p);
        }
    }
    lib3ds_file_free(f);
}


/**
 Load any image file as a texture.
 */
void loadTexture(const char *filename)
{
    Fl_JPEG_Image *img = new Fl_JPEG_Image(filename);
    if (img->d()) {
        texture = img;
    } else {
        delete img;
    }
}

void loadTexture(const char *name, unsigned char *imageDate)
{
    Fl_JPEG_Image *img = new Fl_JPEG_Image(name, imageDate);
    if (img->d()) {
        texture = img;
    } else {
        delete img;
    }
}


static void xButtonCB(Fl_Widget*, void*)
{
    gShowSlice = false;
    glView->redraw();
}

static void sliceCB(Fl_Widget*, void*)
{
    gMeshSlice.generateFrom(gMeshList, zSlider1->value());
    // start a new slice. A slice holds the information from all meshes.
    gMeshSlice.clear();
    // get the number of meshes in this model
    int i, n = (int)gMeshList.size();
    // loop through all meshes
    for (i=0; i<n; i++) {
        IAMesh *IAMesh = gMeshList[i];
        // add all faces in a mesh that intersect with zMin. They will form the lower lid.
        gMeshSlice.addZSlice(*IAMesh, zSlider1->value());
        // use OpenGL to convert the sorted list of edges into a list of simple polygons
        gMeshSlice.tesselate();
    }
    glView->redraw();
}

static void z1ChangedCB(Fl_Widget*, void*)
{
    sliceCB(0, 0);
    gShowSlice = true;
    glView->redraw();
}

static void z2ChangedCB(Fl_Widget*, void*)
{
    gShowSlice = true;
    glView->redraw();
}

static void writeSliceCB(Fl_Widget*, void*)
{
    double z;
#ifdef M_MONKEY
    double firstLayer  = -8.8;
    double lastLayer   =  9.0;
    double layerHeight =  0.1;
    gOutFile = fopen("/Users/matt/monkey.3dp", "wb");
#elif defined M_DRAGON
    double firstLayer  = -20.0;
    double lastLayer   =  14.0;
    double layerHeight =   0.1;
    gOutFile = fopen("/Users/matt/dragon.3dp", "wb");
#elif defined M_PEPSI
    double firstLayer  = -20.0;
    double lastLayer   =  14.0;
    double layerHeight =   0.1;
    gOutFile = fopen("/Users/matt/pepsi.3dp", "wb");
#endif
    // header
    writeInt(gOutFile, 23);  // Magic
    writeInt(gOutFile, 3);
    writeInt(gOutFile, 2013);
    writeInt(gOutFile, 1);   // File Version
    writeInt(gOutFile, 159); // total number of layers
    writeInt(gOutFile,  (lastLayer-firstLayer)/layerHeight );

    for (z=firstLayer; z<=lastLayer; z+=layerHeight) {
        // spread powder
        writeInt(gOutFile, 158);
        writeInt(gOutFile,  10); // spread 0.1mm layers
        // render the layer
        zSlider1->value(z);
        zSlider1->do_callback();
        gWriteSliceNext = 1;
        glView->redraw();
        glView->flush();
        Fl::flush();
    }
    //  writeInt(gOutFile, 158);
    //  writeInt(gOutFile,  25); // spread 0.25mm layers
    //  writeInt(gOutFile, 158);
    //  writeInt(gOutFile,  25); // spread 0.25mm layers
    fclose(gOutFile);
    fprintf(stderr, "/Users/matt/monkey.3dp");
}


static void writePrnSliceCB(Fl_Widget*, void*)
{
    double z;
#ifdef M_MONKEY
    double firstLayer  = -8.8;
    double lastLayer   =  9.0;
    double layerHeight =  0.1;
#elif defined M_DRAGON
    double firstLayer  = -20.0;
    double lastLayer   =  14.0;
    double layerHeight =   0.1;
#elif defined M_PEPSI
    double firstLayer  = -20.0;
    double lastLayer   =  14.0;
    double layerHeight =   0.1;
#endif

    for (z=firstLayer; z<=lastLayer; z+=layerHeight) {
        zSlider1->value(z);
        zSlider1->do_callback();
        gWriteSliceNext = 2;
        glView->redraw();
        glView->flush();
        Fl::flush();
    }
    //  writeInt(gOutFile, 158);
    //  writeInt(gOutFile,  25); // spread 0.25mm layers
    //  writeInt(gOutFile, 158);
    //  writeInt(gOutFile,  25); // spread 0.25mm layers
    fclose(gOutFile);
    fprintf(stderr, "/Users/matt/monkey.3dp");
}


int main (int argc, char **argv)
{
    /*
     500 pixles at 96dpi = 5in = 13cm
     at 96dpi, 1 dot is 0.26mm in diameter
     */
    loadTexture("testcard1024.jpg", defaultTexture);

    Fl_Window win(840, 800, "Iota Slice");
    win.begin();
    Fl_Group *g = new Fl_Group(0, 0, 800, 800);
    g->begin();
    glView = new MyGLView(150, 150, 500, 500);
    g->end();
    zSlider1 = new Fl_Slider(800, 0, 20, 680);
    zSlider1->tooltip("Position of the slice in Z\n-66 to +66 millimeters");
    zSlider1->range(30, -30);
    zSlider1->step(0.25);
    zSlider1->callback(z1ChangedCB);
    zSlider2 = new Fl_Slider(820, 0, 20, 680);
    zSlider2->tooltip("Slice thickness\n-10 to +10 millimeters");
    zSlider2->range(10, -10);
    zSlider2->value(0.25);
    zSlider2->callback(z2ChangedCB);
    Fl_Button *b = new Fl_Button(800, 680, 40, 40, "X");
    b->callback(xButtonCB);
    b = new Fl_Button(800, 720, 40, 40, "Write");
    b->callback(writeSliceCB);
    b = new Fl_Button(800, 760, 40, 40, ".prn");
    b->callback(writePrnSliceCB);
    win.end();
    win.resizable(g);
    win.show(argc, argv);
    glView->show();
    Fl::flush();
#ifdef M_MONKEY
    load3ds("/Users/matt/Desktop/Machine Shop/Machine Pwdr/lib3ds-20080909/monkey.3ds");
#elif defined M_DRAGON
    loadStl("/Users/matt/Desktop/Machine Shop/Machine Pwdr/0.02_dragon_2.stl");
#endif
    //load3ds("/Users/matt/squirrel/NewSquirrel.3ds");
    //load3ds("/Users/matt/Desktop/Machine Shop/Machine Pwdr/0.02_dragon_2.3ds");
    glView->redraw();
    return Fl::run();
}

//
// End of "$Id: hello.cxx 11782 2016-06-14 11:15:22Z AlbrechtS $".
//

