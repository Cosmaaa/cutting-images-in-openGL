//aplicatia arata modul de utilizarea al operatiilor pe pixeli utilizand
//functia glPixelTransfer

#include "glos.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

const double pi = atan(1) * 4;

#pragma pack(1)
typedef struct
{
    GLbyte	identsize;              // Size of ID field that follows header (0)
    GLbyte	colorMapType;           // 0 = None, 1 = paletted
    GLbyte	imageType;              // 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
    unsigned short	colorMapStart;          // First colour map entry
    unsigned short	colorMapLength;         // Number of colors
    unsigned char 	colorMapBits;   // bits per palette entry
    unsigned short	xstart;                 // image x origin
    unsigned short	ystart;                 // image y origin
    unsigned short	width;                  // width in pixels
    unsigned short	height;                 // height in pixels
    GLbyte	bits;                   // bits per pixel (8 16, 24, 32)
    GLbyte	descriptor;             // image descriptor
} TGAHEADER;
#pragma pack(8)


//////////////////////////////////////////////////////////////////
// Module globals to save source image data
static GLbyte *pImage = NULL;
static GLint iWidth, iHeight, iComponents;
static GLenum eFormat;

// Global variable to store desired drawing mode
static GLint    iRenderMode = 1;    

GLint gltWriteTGA(const char *szFileName);
GLbyte *gltLoadTGA(const char *szFileName, GLint *iWidth, GLint *iHeight, GLint *iComponents, GLenum *eFormat);

GLbyte *gltLoadTGA(const char *szFileName, GLint *iWidth, GLint *iHeight, GLint *iComponents, GLenum *eFormat)
	{
    FILE *pFile;			// File pointer
    TGAHEADER tgaHeader;		// TGA file header
    unsigned long lImageSize;		// Size in bytes of image
    short sDepth;			// Pixel depth;
    GLbyte	*pBits = NULL;          // Pointer to bits
    
    // Default/Failed values
    *iWidth = 0;
    *iHeight = 0;
    *eFormat = GL_BGR_EXT;
    *iComponents = GL_RGB8;
    
    // Attempt to open the file
    pFile = fopen(szFileName, "rb");
    if(pFile == NULL)
        return NULL;
	
    // Read in header (binary)
    fread(&tgaHeader, sizeof(TGAHEADER), 1, pFile);
    

    // Get width, height, and depth of texture
    *iWidth = tgaHeader.width;
    *iHeight = tgaHeader.height;
    sDepth = tgaHeader.bits / 8;
    printf ("a citit fisierul %d\n",tgaHeader.height);
    // Put some validity checks here. Very simply, I only understand
    // or care about 8, 24, or 32 bit targa's.
   // if(tgaHeader.bits != 8 && tgaHeader.bits != 24 && tgaHeader.bits != 32)
   //    return NULL;
	
    // Calculate size of image buffer
    lImageSize = tgaHeader.width * tgaHeader.height * sDepth;
    printf ("a citit fisierul %d\n",lImageSize);
    // Allocate memory and check for success
    pBits = (GLbyte*)malloc(lImageSize * sizeof(GLbyte));
    if(pBits == NULL)
        return NULL;
    
    // Read in the bits
    // Check for read error. This should catch RLE or other 
    // weird formats that I don't want to recognize
    if(fread(pBits, lImageSize, 1, pFile) != 1)
		{
        free(pBits);
        return NULL;
		}
    
    // Set OpenGL format expected
    switch(sDepth)
		{
        case 3:     // Most likely case
            *eFormat = GL_BGR_EXT;
            *iComponents = GL_RGB8;
            break;
        case 4:
            *eFormat = GL_BGRA_EXT;
            *iComponents = GL_RGBA8;
            break;
        case 1:
            *eFormat = GL_LUMINANCE;
            *iComponents = GL_LUMINANCE8;
            break;
		};
	
    
    // Done with File
    fclose(pFile);
	
    // Return pointer to image data
    return pBits;
	}

GLint gltWriteTGA(const char *szFileName)
	{
    FILE *pFile;                // File pointer
    TGAHEADER tgaHeader;		// TGA file header
    unsigned long lImageSize;   // Size in bytes of image
    GLbyte	*pBits = NULL;      // Pointer to bits
    GLint iViewport[4];         // Viewport in pixels
    GLenum lastBuffer;          // Storage for the current read buffer setting
    
	// Get the viewport dimensions
	glGetIntegerv(GL_VIEWPORT, iViewport);
	
    // How big is the image going to be (targas are tightly packed)
	lImageSize = iViewport[2] * 3 * iViewport[3];	
	
    // Allocate block. If this doesn't work, go home
    pBits = (GLbyte *)malloc(lImageSize);
    if(pBits == NULL)
        return 0;
	
    // Read bits from color buffer
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    
    // Get the current read buffer setting and save it. Switch to
    // the front buffer and do the read operation. Finally, restore
    // the read buffer state
    glGetIntegerv(GL_READ_BUFFER, (GLint *)&lastBuffer);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, iViewport[2], iViewport[3], GL_BGR_EXT, GL_UNSIGNED_BYTE, pBits);
    glReadBuffer(lastBuffer);
    
    // Initialize the Targa header
    tgaHeader.identsize = 0;
    tgaHeader.colorMapType = 0;
    tgaHeader.imageType = 2;
    tgaHeader.colorMapStart = 0;
    tgaHeader.colorMapLength = 0;
    tgaHeader.colorMapBits = 0;
    tgaHeader.xstart = 0;
    tgaHeader.ystart = 0;
    tgaHeader.width = iViewport[2];
    tgaHeader.height = iViewport[3];
    tgaHeader.bits = 24;
    tgaHeader.descriptor = 0;
    
        
    // Attempt to open the file
    pFile = fopen(szFileName, "wb");
    if(pFile == NULL)
		{
        free(pBits);    // Free buffer and return error
        return 0;
		}
	
    // Write the header
    fwrite(&tgaHeader, sizeof(TGAHEADER), 1, pFile);
    
    // Write the image data
    fwrite(pBits, lImageSize, 1, pFile);
	
    // Free temporary buffer and close the file
    free(pBits);    
    fclose(pFile);
    
    // Success!
    return 1;
	}
void drawTriangleStencil() {
    glBegin(GL_TRIANGLES);
    // Coordonatele triunghiului
    glVertex2f(0.5f, 1.0f); // Vârful
    glVertex2f(0.0f, 0.0f); // Stânga jos
    glVertex2f(1.0f, 0.0f); // Dreapta jos
    glEnd();
}

void drawCircleStencil(float radius, int segments) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0, 0); // Centrul cercului
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * pi * float(i) / float(segments);
        glVertex2f(cos(angle) * radius, sin(angle) * radius);
    }
    glEnd();
}
        
//////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering
// context. 
void SetupRC(void)
    {
    // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
    // Load the horse image
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   	pImage = gltLoadTGA("horse.tga", &iWidth, &iHeight, &iComponents, &eFormat);
    }

void ShutdownRC(void)
    {
    // Free the original image data
    free(pImage);
    }


///////////////////////////////////////////////////////////////////////////////
// Reset flags as appropriate in response to menu selections
void ProcessMenu(int value)
	{
    if(value == 0)
	     // Save image
        gltWriteTGA("ScreenShot.tga");
    else
        // Change render mode index to match menu entry index
        iRenderMode = value;
        
    // Trigger Redraw
	glutPostRedisplay();
	}



///////////////////////////////////////////////////////////////////////        
// Called to draw scene
void RenderScene(void)
    {
    GLint iViewport[4];
    GLbyte *pModifiedBytes = NULL;
    GLfloat invertMap[256];
    GLint i;
    
    // Clear the window with current clearing color
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Current Raster Position always at bottom left hand corner of window
    glRasterPos2i(0, 0);
        
    // Do image operation, depending on rendermode index
    switch (iRenderMode) {
    case 1:     // Decupare în formă de triunghi
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glClear(GL_STENCIL_BUFFER_BIT);
        drawTriangleStencil();
        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glDrawPixels(iWidth, iHeight, eFormat, GL_UNSIGNED_BYTE, pImage);
        glDisable(GL_STENCIL_TEST);
        break;

    case 2:     // Decupare în formă de cerc
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glClear(GL_STENCIL_BUFFER_BIT);
        drawCircleStencil(0.5f, 100); // Raza și numărul de segmente
        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glDrawPixels(iWidth, iHeight, eFormat, GL_UNSIGNED_BYTE, pImage);
        glDisable(GL_STENCIL_TEST);
        break;

        // Adaugă aici cazul pentru decuparea în formă de poligon

    default:
        glDrawPixels(iWidth, iHeight, eFormat, GL_UNSIGNED_BYTE, pImage);
        break;
    }

    glutSwapBuffers();
}
		



void ChangeSize(int w, int h)
    {
    // Prevent a divide by zero, when window is too short
    // (you cant make a window of zero width).
    if(h == 0)
        h = 1;

    glViewport(0, 0, w, h);
        
	// Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
    // Set the clipping volume
    gluOrtho2D(0.0f, (GLfloat) w, 0.0, (GLfloat) h);
        
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();    
    }

/////////////////////////////////////////////////////////////
// Main program entrypoint
int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GL_DOUBLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenGL Image Operations");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);

    glutCreateMenu(ProcessMenu);
    glutAddMenuEntry("Cut Triangle", 1);
    glutAddMenuEntry("Cut Circle", 2);
    // Adaugă aici meniul pentru decuparea în formă de poligon
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    SetupRC();
    glutMainLoop();
    ShutdownRC();
    return 0;
}
